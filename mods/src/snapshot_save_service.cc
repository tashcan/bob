#include "snapshot_save_service.h"
#include <limits>
#include <stdexcept>
#if defined(_WIN32)
#include <Windows.h>
#endif

namespace persistence
{
namespace
{
std::atomic_flag serviceOwned = ATOMIC_FLAG_INIT;
std::atomic<std::uint64_t> lastSession{0};
#if defined(MOD_SNAPSHOT_SERVICE_TESTING)
void BeforeServiceWorkerStart(std::size_t index);
#endif
std::uint64_t NewSession()
{
  auto previous = lastSession.load();
  do {
    if (previous == (std::numeric_limits<std::uint64_t>::max)())
      throw std::overflow_error("snapshot service session exhausted");
  } while (!lastSession.compare_exchange_weak(previous, previous + 1));
  return previous + 1;
}

bool AmbiguousNames(const std::filesystem::path& first, const std::filesystem::path& second)
{
#if defined(_WIN32)
  const auto a = first.native(), b = second.native();
  if (a.size() > INT_MAX || b.size() > INT_MAX)
    return true;
  const int comparison = CompareStringOrdinal(a.data(), static_cast<int>(a.size()), b.data(),
                                               static_cast<int>(b.size()), TRUE);
  return comparison == 0 || comparison == CSTR_EQUAL;
#else
  // Conservative for case/normalization-sensitive volume differences. Existing
  // aliases also use filesystem equivalence. For absent Unicode leaves in the
  // same directory, refuse ambiguity rather than guess a filesystem collation.
  auto a = first.native(), b = second.native();
  for (auto* text : {&a, &b}) {
    for (char& c : *text) {
      if (static_cast<unsigned char>(c) >= 128)
        return true;
      if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
    }
  }
  return a == b;
#endif
}
void ValidateSpelling(const std::filesystem::path& path)
{
#if defined(_WIN32)
  // Win32 strips these suffixes on target access but not when a suffix such as
  // .lock follows them. Reject before canonicalization can hide that spelling.
  for (const auto& component : path) {
    const auto& text = component.native();
    if (text != L"." && text != L".." && !text.empty() && (text.back() == L'.' || text.back() == L' '))
      throw std::invalid_argument("ambiguous Windows snapshot path spelling");
  }
#else
  (void)path;
#endif
}
bool SameIdentity(const std::filesystem::path& first, const std::filesystem::path& second)
{
  return (std::filesystem::equivalent(first.parent_path(), second.parent_path()) &&
          AmbiguousNames(first.filename(), second.filename())) ||
         (std::filesystem::exists(first) && std::filesystem::exists(second) &&
          std::filesystem::equivalent(first, second));
}
}

SnapshotSaveService::Lease::Lease()
{
  if (serviceOwned.test_and_set())
    throw std::logic_error("snapshot service already owned");
}
SnapshotSaveService::Lease::~Lease() { serviceOwned.clear(); }

SnapshotSaveService::SnapshotSaveService(std::span<const std::filesystem::path> paths,
                                       const std::atomic_bool* hostCancellation)
    : session_(NewSession()), count_(paths.size())
{
  if (count_ == 0 || count_ > MaxDestinations)
    throw std::invalid_argument("snapshot destination count out of bounds");
  std::array<std::filesystem::path, MaxDestinations> resolved;
  // Finish filesystem enrollment before any thread starts. Existing parents are
  // required; this registry creates no paths or config files.
  for (std::size_t i = 0; i < count_; ++i) {
    if (paths[i].empty() || paths[i].native().find(std::filesystem::path::value_type{}) !=
                              std::filesystem::path::string_type::npos)
      throw std::invalid_argument("invalid snapshot destination");
    ValidateSpelling(paths[i]);
    resolved[i] = std::filesystem::weakly_canonical(std::filesystem::absolute(paths[i]));
    ValidateSpelling(resolved[i]);
    if (!std::filesystem::is_directory(resolved[i].parent_path()) || resolved[i].filename().empty())
      throw std::invalid_argument("snapshot parent must exist");
    const bool exists = std::filesystem::exists(resolved[i]);
#if defined(_WIN32)
    // On 8.3-enabled volumes another destination's creation can turn an absent
    // tilde leaf into its short alias. Reject conservatively without depending
    // on the current volume policy. Existing aliases use equivalence below.
    if (!exists && resolved[i].filename().native().find(L'~') != std::wstring::npos)
      throw std::invalid_argument("ambiguous absent Windows short-name destination");
#endif
    if (exists && (!std::filesystem::is_regular_file(resolved[i]) || std::filesystem::hard_link_count(resolved[i]) != 1))
      throw std::invalid_argument("snapshot destination must be a single regular file");
    for (std::size_t j = 0; j < i; ++j) {
      auto lockI = resolved[i], lockJ = resolved[j];
      lockI += ".lock";
      lockJ += ".lock";
      // A destination must never replace another transaction's lock inode.
      // Reserve the complete target/lock identity pair, in either order.
      if (SameIdentity(resolved[i], resolved[j]) || SameIdentity(resolved[i], lockJ) ||
          SameIdentity(lockI, resolved[j]) || SameIdentity(lockI, lockJ))
        throw std::invalid_argument("duplicate or ambiguous snapshot destination or lock");
    }
  }
  try {
    for (std::size_t i = 0; i < count_; ++i) {
#if defined(MOD_SNAPSHOT_SERVICE_TESTING)
      BeforeServiceWorkerStart(i);
#endif
      workers_[i] = std::make_unique<SnapshotSaveWorker>(resolved[i], hostCancellation);
    }
  } catch (...) {
    // Construction is supervisor/startup-only. Partial thread creation must not
    // destroy a joinable worker during stack unwinding.
    for (auto& worker : workers_)
      if (worker) worker->StopAndJoin(StopMode::CancelQueued);
    throw;
  }
}

bool SnapshotSaveService::Valid(Destination destination) const noexcept
{ return destination.session_ == session_ && destination.index_ < count_; }

SnapshotSaveService::Destination SnapshotSaveService::GetDestination(std::size_t index) const
{
  if (index >= count_) throw std::out_of_range("snapshot destination index");
  Destination result;
  result.session_ = session_;
  result.index_ = index;
  return result;
}

SnapshotSaveQueue::Submission SnapshotSaveService::TrySubmit(Destination destination, std::uint64_t revision,
                                                           std::string&& bytes)
{
  if (!Valid(destination)) return {SnapshotSaveQueue::Admission::InvalidRequest};
  if (stopping_.load()) return {SnapshotSaveQueue::Admission::Stopping};
  return workers_[destination.index_]->TrySubmit(revision, std::move(bytes));
}
std::optional<SnapshotSaveQueue::Completion> SnapshotSaveService::TryTakeCompletion(Destination destination)
{
  if (!Valid(destination)) return std::nullopt;
  return workers_[destination.index_]->TryTakeCompletion();
}
void SnapshotSaveService::RequestStop(StopMode mode) noexcept
{
  stopping_.store(true);
  for (std::size_t i = 0; i < count_; ++i) workers_[i]->RequestStop(mode);
}
void SnapshotSaveService::StopAndJoin(StopMode mode)
{
  RequestStop(mode); // Close every destination before joining any stalled one.
  for (std::size_t i = 0; i < count_; ++i) workers_[i]->StopAndJoin(mode);
}
}
