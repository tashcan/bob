// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: Digit.Prime.NewFlags.proto

#include "Digit.Prime.NewFlags.pb.h"

#include <algorithm>
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/extension_set.h"
#include "google/protobuf/wire_format_lite.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/generated_message_tctable_impl.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"
PROTOBUF_PRAGMA_INIT_SEG
namespace _pb = ::google::protobuf;
namespace _pbi = ::google::protobuf::internal;
namespace _fl = ::google::protobuf::internal::field_layout;
namespace Digit {
namespace Prime {
namespace NewFlags {
      template <typename>
PROTOBUF_CONSTEXPR NewFlagCacheMap_ItemsEntry_DoNotUse::NewFlagCacheMap_ItemsEntry_DoNotUse(::_pbi::ConstantInitialized) {}
struct NewFlagCacheMap_ItemsEntry_DoNotUseDefaultTypeInternal {
  PROTOBUF_CONSTEXPR NewFlagCacheMap_ItemsEntry_DoNotUseDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~NewFlagCacheMap_ItemsEntry_DoNotUseDefaultTypeInternal() {}
  union {
    NewFlagCacheMap_ItemsEntry_DoNotUse _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 NewFlagCacheMap_ItemsEntry_DoNotUseDefaultTypeInternal _NewFlagCacheMap_ItemsEntry_DoNotUse_default_instance_;
      template <typename>
PROTOBUF_CONSTEXPR NewFlagCacheMap_ItemFlagsEntry_DoNotUse::NewFlagCacheMap_ItemFlagsEntry_DoNotUse(::_pbi::ConstantInitialized) {}
struct NewFlagCacheMap_ItemFlagsEntry_DoNotUseDefaultTypeInternal {
  PROTOBUF_CONSTEXPR NewFlagCacheMap_ItemFlagsEntry_DoNotUseDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~NewFlagCacheMap_ItemFlagsEntry_DoNotUseDefaultTypeInternal() {}
  union {
    NewFlagCacheMap_ItemFlagsEntry_DoNotUse _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 NewFlagCacheMap_ItemFlagsEntry_DoNotUseDefaultTypeInternal _NewFlagCacheMap_ItemFlagsEntry_DoNotUse_default_instance_;
        template <typename>
PROTOBUF_CONSTEXPR NewFlagCacheMap::NewFlagCacheMap(::_pbi::ConstantInitialized)
    : _impl_{
      /* decltype(_impl_.items_) */ {},
      /* decltype(_impl_.itemflags_) */ {},
      /*decltype(_impl_._cached_size_)*/ {},
    } {}
struct NewFlagCacheMapDefaultTypeInternal {
  PROTOBUF_CONSTEXPR NewFlagCacheMapDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~NewFlagCacheMapDefaultTypeInternal() {}
  union {
    NewFlagCacheMap _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 NewFlagCacheMapDefaultTypeInternal _NewFlagCacheMap_default_instance_;
      template <typename>
PROTOBUF_CONSTEXPR NewFlagDataCache_CategoriesEntry_DoNotUse::NewFlagDataCache_CategoriesEntry_DoNotUse(::_pbi::ConstantInitialized) {}
struct NewFlagDataCache_CategoriesEntry_DoNotUseDefaultTypeInternal {
  PROTOBUF_CONSTEXPR NewFlagDataCache_CategoriesEntry_DoNotUseDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~NewFlagDataCache_CategoriesEntry_DoNotUseDefaultTypeInternal() {}
  union {
    NewFlagDataCache_CategoriesEntry_DoNotUse _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 NewFlagDataCache_CategoriesEntry_DoNotUseDefaultTypeInternal _NewFlagDataCache_CategoriesEntry_DoNotUse_default_instance_;
        template <typename>
PROTOBUF_CONSTEXPR NewFlagDataCache::NewFlagDataCache(::_pbi::ConstantInitialized)
    : _impl_{
      /* decltype(_impl_.categories_) */ {},
      /*decltype(_impl_.version_)*/ 0,
      /*decltype(_impl_._cached_size_)*/ {},
    } {}
struct NewFlagDataCacheDefaultTypeInternal {
  PROTOBUF_CONSTEXPR NewFlagDataCacheDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~NewFlagDataCacheDefaultTypeInternal() {}
  union {
    NewFlagDataCache _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 NewFlagDataCacheDefaultTypeInternal _NewFlagDataCache_default_instance_;
}  // namespace NewFlags
}  // namespace Prime
}  // namespace Digit
namespace Digit {
namespace Prime {
namespace NewFlags {
// ===================================================================

NewFlagCacheMap_ItemsEntry_DoNotUse::NewFlagCacheMap_ItemsEntry_DoNotUse() {}
NewFlagCacheMap_ItemsEntry_DoNotUse::NewFlagCacheMap_ItemsEntry_DoNotUse(::google::protobuf::Arena* arena)
    : SuperType(arena) {}
void NewFlagCacheMap_ItemsEntry_DoNotUse::MergeFrom(const NewFlagCacheMap_ItemsEntry_DoNotUse& other) {
  MergeFromInternal(other);
}
// ===================================================================

NewFlagCacheMap_ItemFlagsEntry_DoNotUse::NewFlagCacheMap_ItemFlagsEntry_DoNotUse() {}
NewFlagCacheMap_ItemFlagsEntry_DoNotUse::NewFlagCacheMap_ItemFlagsEntry_DoNotUse(::google::protobuf::Arena* arena)
    : SuperType(arena) {}
void NewFlagCacheMap_ItemFlagsEntry_DoNotUse::MergeFrom(const NewFlagCacheMap_ItemFlagsEntry_DoNotUse& other) {
  MergeFromInternal(other);
}
// ===================================================================

class NewFlagCacheMap::_Internal {
 public:
};

NewFlagCacheMap::NewFlagCacheMap(::google::protobuf::Arena* arena)
    : ::google::protobuf::MessageLite(arena) {
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:Digit.Prime.NewFlags.NewFlagCacheMap)
}
NewFlagCacheMap::NewFlagCacheMap(const NewFlagCacheMap& from) : ::google::protobuf::MessageLite() {
  NewFlagCacheMap* const _this = this;
  (void)_this;
  new (&_impl_) Impl_{
      /* decltype(_impl_.items_) */ {},
      /* decltype(_impl_.itemflags_) */ {},
      /*decltype(_impl_._cached_size_)*/ {},
  };
  _internal_metadata_.MergeFrom<std::string>(
      from._internal_metadata_);
  _this->_impl_.items_.MergeFrom(from._impl_.items_);
  _this->_impl_.itemflags_.MergeFrom(from._impl_.itemflags_);

  // @@protoc_insertion_point(copy_constructor:Digit.Prime.NewFlags.NewFlagCacheMap)
}
inline void NewFlagCacheMap::SharedCtor(::_pb::Arena* arena) {
  (void)arena;
  new (&_impl_) Impl_{
      /* decltype(_impl_.items_) */ {::google::protobuf::internal::ArenaInitialized(), arena},
      /* decltype(_impl_.itemflags_) */ {::google::protobuf::internal::ArenaInitialized(), arena},
      /*decltype(_impl_._cached_size_)*/ {},
  };
}
NewFlagCacheMap::~NewFlagCacheMap() {
  // @@protoc_insertion_point(destructor:Digit.Prime.NewFlags.NewFlagCacheMap)
  _internal_metadata_.Delete<std::string>();
  SharedDtor();
}
inline void NewFlagCacheMap::SharedDtor() {
  ABSL_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.items_.~MapFieldLite();
  _impl_.itemflags_.~MapFieldLite();
}
void NewFlagCacheMap::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

PROTOBUF_NOINLINE void NewFlagCacheMap::Clear() {
// @@protoc_insertion_point(message_clear_start:Digit.Prime.NewFlags.NewFlagCacheMap)
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.items_.Clear();
  _impl_.itemflags_.Clear();
  _internal_metadata_.Clear<std::string>();
}

const char* NewFlagCacheMap::_InternalParse(
    const char* ptr, ::_pbi::ParseContext* ctx) {
  ptr = ::_pbi::TcParser::ParseLoop(this, ptr, ctx, &_table_.header);
  return ptr;
}


PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<0, 2, 2, 0, 2> NewFlagCacheMap::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    2, 0,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967292,  // skipmap
    offsetof(decltype(_table_), field_entries),
    2,  // num_field_entries
    2,  // num_aux_entries
    offsetof(decltype(_table_), aux_entries),
    &_NewFlagCacheMap_default_instance_._instance,
    ::_pbi::TcParser::GenericFallbackLite,  // fallback
  }, {{
    {::_pbi::TcParser::MiniParse, {}},
  }}, {{
    65535, 65535
  }}, {{
    // map<int64, bool> items = 1;
    {PROTOBUF_FIELD_OFFSET(NewFlagCacheMap, _impl_.items_), 0, 0,
    (0 | ::_fl::kFcRepeated | ::_fl::kMap)},
    // map<int64, int32> itemFlags = 2;
    {PROTOBUF_FIELD_OFFSET(NewFlagCacheMap, _impl_.itemflags_), 0, 1,
    (0 | ::_fl::kFcRepeated | ::_fl::kMap)},
  }}, {{
    {::_pbi::TcParser::GetMapAuxInfo<decltype(NewFlagCacheMap()._impl_.items_)>(1, 0, 0)},
    {::_pbi::TcParser::GetMapAuxInfo<decltype(NewFlagCacheMap()._impl_.itemflags_)>(1, 0, 0)},
  }}, {{
  }},
};

::uint8_t* NewFlagCacheMap::_InternalSerialize(
    ::uint8_t* target,
    ::google::protobuf::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:Digit.Prime.NewFlags.NewFlagCacheMap)
  ::uint32_t cached_has_bits = 0;
  (void)cached_has_bits;

  // map<int64, bool> items = 1;
  if (!_internal_items().empty()) {
    using MapType = ::google::protobuf::Map<::int64_t, bool>;
    using WireHelper = NewFlagCacheMap_ItemsEntry_DoNotUse::Funcs;
    const auto& field = _internal_items();

    if (stream->IsSerializationDeterministic() && field.size() > 1) {
      for (const auto& entry : ::google::protobuf::internal::MapSorterFlat<MapType>(field)) {
        target = WireHelper::InternalSerialize(
            1, entry.first, entry.second, target, stream);
      }
    } else {
      for (const auto& entry : field) {
        target = WireHelper::InternalSerialize(
            1, entry.first, entry.second, target, stream);
      }
    }
  }

  // map<int64, int32> itemFlags = 2;
  if (!_internal_itemflags().empty()) {
    using MapType = ::google::protobuf::Map<::int64_t, ::int32_t>;
    using WireHelper = NewFlagCacheMap_ItemFlagsEntry_DoNotUse::Funcs;
    const auto& field = _internal_itemflags();

    if (stream->IsSerializationDeterministic() && field.size() > 1) {
      for (const auto& entry : ::google::protobuf::internal::MapSorterFlat<MapType>(field)) {
        target = WireHelper::InternalSerialize(
            2, entry.first, entry.second, target, stream);
      }
    } else {
      for (const auto& entry : field) {
        target = WireHelper::InternalSerialize(
            2, entry.first, entry.second, target, stream);
      }
    }
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = stream->WriteRaw(
        _internal_metadata_.unknown_fields<std::string>(::google::protobuf::internal::GetEmptyString).data(),
        static_cast<int>(_internal_metadata_.unknown_fields<std::string>(::google::protobuf::internal::GetEmptyString).size()), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:Digit.Prime.NewFlags.NewFlagCacheMap)
  return target;
}

::size_t NewFlagCacheMap::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:Digit.Prime.NewFlags.NewFlagCacheMap)
  ::size_t total_size = 0;

  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // map<int64, bool> items = 1;
  total_size += 1 * ::google::protobuf::internal::FromIntSize(_internal_items_size());
  for (const auto& entry : _internal_items()) {
    total_size += NewFlagCacheMap_ItemsEntry_DoNotUse::Funcs::ByteSizeLong(entry.first, entry.second);
  }
  // map<int64, int32> itemFlags = 2;
  total_size += 1 * ::google::protobuf::internal::FromIntSize(_internal_itemflags_size());
  for (const auto& entry : _internal_itemflags()) {
    total_size += NewFlagCacheMap_ItemFlagsEntry_DoNotUse::Funcs::ByteSizeLong(entry.first, entry.second);
  }
  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    total_size += _internal_metadata_.unknown_fields<std::string>(::google::protobuf::internal::GetEmptyString).size();
  }
  int cached_size = ::_pbi::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void NewFlagCacheMap::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::_pbi::DownCast<const NewFlagCacheMap*>(
      &from));
}

void NewFlagCacheMap::MergeFrom(const NewFlagCacheMap& from) {
  NewFlagCacheMap* const _this = this;
  // @@protoc_insertion_point(class_specific_merge_from_start:Digit.Prime.NewFlags.NewFlagCacheMap)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_impl_.items_.MergeFrom(from._impl_.items_);
  _this->_impl_.itemflags_.MergeFrom(from._impl_.itemflags_);
  _this->_internal_metadata_.MergeFrom<std::string>(from._internal_metadata_);
}

void NewFlagCacheMap::CopyFrom(const NewFlagCacheMap& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:Digit.Prime.NewFlags.NewFlagCacheMap)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

PROTOBUF_NOINLINE bool NewFlagCacheMap::IsInitialized() const {
  return true;
}

void NewFlagCacheMap::InternalSwap(NewFlagCacheMap* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.items_.InternalSwap(&other->_impl_.items_);
  _impl_.itemflags_.InternalSwap(&other->_impl_.itemflags_);
}

std::string NewFlagCacheMap::GetTypeName() const {
  return "Digit.Prime.NewFlags.NewFlagCacheMap";
}

// ===================================================================

NewFlagDataCache_CategoriesEntry_DoNotUse::NewFlagDataCache_CategoriesEntry_DoNotUse() {}
NewFlagDataCache_CategoriesEntry_DoNotUse::NewFlagDataCache_CategoriesEntry_DoNotUse(::google::protobuf::Arena* arena)
    : SuperType(arena) {}
void NewFlagDataCache_CategoriesEntry_DoNotUse::MergeFrom(const NewFlagDataCache_CategoriesEntry_DoNotUse& other) {
  MergeFromInternal(other);
}
// ===================================================================

class NewFlagDataCache::_Internal {
 public:
};

NewFlagDataCache::NewFlagDataCache(::google::protobuf::Arena* arena)
    : ::google::protobuf::MessageLite(arena) {
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:Digit.Prime.NewFlags.NewFlagDataCache)
}
NewFlagDataCache::NewFlagDataCache(const NewFlagDataCache& from) : ::google::protobuf::MessageLite() {
  NewFlagDataCache* const _this = this;
  (void)_this;
  new (&_impl_) Impl_{
      /* decltype(_impl_.categories_) */ {},
      decltype(_impl_.version_){},
      /*decltype(_impl_._cached_size_)*/ {},
  };
  _internal_metadata_.MergeFrom<std::string>(
      from._internal_metadata_);
  _this->_impl_.categories_.MergeFrom(from._impl_.categories_);
  _this->_impl_.version_ = from._impl_.version_;

  // @@protoc_insertion_point(copy_constructor:Digit.Prime.NewFlags.NewFlagDataCache)
}
inline void NewFlagDataCache::SharedCtor(::_pb::Arena* arena) {
  (void)arena;
  new (&_impl_) Impl_{
      /* decltype(_impl_.categories_) */ {::google::protobuf::internal::ArenaInitialized(), arena},
      decltype(_impl_.version_){0},
      /*decltype(_impl_._cached_size_)*/ {},
  };
}
NewFlagDataCache::~NewFlagDataCache() {
  // @@protoc_insertion_point(destructor:Digit.Prime.NewFlags.NewFlagDataCache)
  _internal_metadata_.Delete<std::string>();
  SharedDtor();
}
inline void NewFlagDataCache::SharedDtor() {
  ABSL_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.categories_.~MapFieldLite();
}
void NewFlagDataCache::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

PROTOBUF_NOINLINE void NewFlagDataCache::Clear() {
// @@protoc_insertion_point(message_clear_start:Digit.Prime.NewFlags.NewFlagDataCache)
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.categories_.Clear();
  _impl_.version_ = 0;
  _internal_metadata_.Clear<std::string>();
}

const char* NewFlagDataCache::_InternalParse(
    const char* ptr, ::_pbi::ParseContext* ctx) {
  ptr = ::_pbi::TcParser::ParseLoop(this, ptr, ctx, &_table_.header);
  return ptr;
}


PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<0, 2, 2, 0, 2> NewFlagDataCache::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    2, 0,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967292,  // skipmap
    offsetof(decltype(_table_), field_entries),
    2,  // num_field_entries
    2,  // num_aux_entries
    offsetof(decltype(_table_), aux_entries),
    &_NewFlagDataCache_default_instance_._instance,
    ::_pbi::TcParser::GenericFallbackLite,  // fallback
  }, {{
    // int32 version = 2;
    {::_pbi::TcParser::FastV32S1,
     {16, 63, 0, PROTOBUF_FIELD_OFFSET(NewFlagDataCache, _impl_.version_)}},
  }}, {{
    65535, 65535
  }}, {{
    // map<int32, .Digit.Prime.NewFlags.NewFlagCacheMap> categories = 1;
    {PROTOBUF_FIELD_OFFSET(NewFlagDataCache, _impl_.categories_), 0, 0,
    (0 | ::_fl::kFcRepeated | ::_fl::kMap)},
    // int32 version = 2;
    {PROTOBUF_FIELD_OFFSET(NewFlagDataCache, _impl_.version_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kInt32)},
  }}, {{
    {::_pbi::TcParser::GetMapAuxInfo<decltype(NewFlagDataCache()._impl_.categories_)>(1, 0, 0)},
    {::_pbi::TcParser::CreateInArenaStorageCb<::Digit::Prime::NewFlags::NewFlagCacheMap>},
  }}, {{
  }},
};

::uint8_t* NewFlagDataCache::_InternalSerialize(
    ::uint8_t* target,
    ::google::protobuf::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:Digit.Prime.NewFlags.NewFlagDataCache)
  ::uint32_t cached_has_bits = 0;
  (void)cached_has_bits;

  // map<int32, .Digit.Prime.NewFlags.NewFlagCacheMap> categories = 1;
  if (!_internal_categories().empty()) {
    using MapType = ::google::protobuf::Map<::int32_t, ::Digit::Prime::NewFlags::NewFlagCacheMap>;
    using WireHelper = NewFlagDataCache_CategoriesEntry_DoNotUse::Funcs;
    const auto& field = _internal_categories();

    if (stream->IsSerializationDeterministic() && field.size() > 1) {
      for (const auto& entry : ::google::protobuf::internal::MapSorterFlat<MapType>(field)) {
        target = WireHelper::InternalSerialize(
            1, entry.first, entry.second, target, stream);
      }
    } else {
      for (const auto& entry : field) {
        target = WireHelper::InternalSerialize(
            1, entry.first, entry.second, target, stream);
      }
    }
  }

  // int32 version = 2;
  if (this->_internal_version() != 0) {
    target = ::google::protobuf::internal::WireFormatLite::
        WriteInt32ToArrayWithField<2>(
            stream, this->_internal_version(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = stream->WriteRaw(
        _internal_metadata_.unknown_fields<std::string>(::google::protobuf::internal::GetEmptyString).data(),
        static_cast<int>(_internal_metadata_.unknown_fields<std::string>(::google::protobuf::internal::GetEmptyString).size()), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:Digit.Prime.NewFlags.NewFlagDataCache)
  return target;
}

::size_t NewFlagDataCache::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:Digit.Prime.NewFlags.NewFlagDataCache)
  ::size_t total_size = 0;

  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // map<int32, .Digit.Prime.NewFlags.NewFlagCacheMap> categories = 1;
  total_size += 1 * ::google::protobuf::internal::FromIntSize(_internal_categories_size());
  for (const auto& entry : _internal_categories()) {
    total_size += NewFlagDataCache_CategoriesEntry_DoNotUse::Funcs::ByteSizeLong(entry.first, entry.second);
  }
  // int32 version = 2;
  if (this->_internal_version() != 0) {
    total_size += ::_pbi::WireFormatLite::Int32SizePlusOne(
        this->_internal_version());
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    total_size += _internal_metadata_.unknown_fields<std::string>(::google::protobuf::internal::GetEmptyString).size();
  }
  int cached_size = ::_pbi::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void NewFlagDataCache::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::_pbi::DownCast<const NewFlagDataCache*>(
      &from));
}

void NewFlagDataCache::MergeFrom(const NewFlagDataCache& from) {
  NewFlagDataCache* const _this = this;
  // @@protoc_insertion_point(class_specific_merge_from_start:Digit.Prime.NewFlags.NewFlagDataCache)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_impl_.categories_.MergeFrom(from._impl_.categories_);
  if (from._internal_version() != 0) {
    _this->_internal_set_version(from._internal_version());
  }
  _this->_internal_metadata_.MergeFrom<std::string>(from._internal_metadata_);
}

void NewFlagDataCache::CopyFrom(const NewFlagDataCache& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:Digit.Prime.NewFlags.NewFlagDataCache)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

PROTOBUF_NOINLINE bool NewFlagDataCache::IsInitialized() const {
  return true;
}

void NewFlagDataCache::InternalSwap(NewFlagDataCache* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.categories_.InternalSwap(&other->_impl_.categories_);
        swap(_impl_.version_, other->_impl_.version_);
}

std::string NewFlagDataCache::GetTypeName() const {
  return "Digit.Prime.NewFlags.NewFlagDataCache";
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace NewFlags
}  // namespace Prime
}  // namespace Digit
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google
// @@protoc_insertion_point(global_scope)
#include "google/protobuf/port_undef.inc"
