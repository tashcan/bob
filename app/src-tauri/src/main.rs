// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use futures_util::{future::join_all, FutureExt};
use tauri::Window;
use winapi::ctypes::c_void;

#[tauri::command]
fn game_installed() -> bool {
    let game_path = get_game_path();
    if let Some(game_path) = game_path {
        std::path::Path::new(&game_path).is_dir()
            && std::path::Path::new(&game_path).join(".version").exists()
    } else {
        false
    }
}

#[tauri::command]
fn get_game_path() -> Option<String> {
    let local_app_data = dirs::data_local_dir();
    if let Some(local_app_data) = local_app_data {
        let ini_path = local_app_data
            .join("Star Trek Fleet Command")
            .join("launcher_settings.ini");
        let ini_data = std::fs::read_to_string(ini_path).ok()?;
        for line in ini_data.lines() {
            if line.starts_with("152033..GAME_PATH") {
                let game_path = line.split_once('=')?.1;
                return Some(game_path.to_string());
            }
        }
    }
    None
}

struct HandleWrapper(*mut c_void);
unsafe impl Send for HandleWrapper {}
unsafe impl Sync for HandleWrapper {}

fn wait(handle: HandleWrapper) -> u32 {
    unsafe { winapi::um::synchapi::WaitForSingleObject(handle.0, winapi::um::winbase::INFINITE) }
}

fn get_patch_data_path() -> std::path::PathBuf {
    let local_app_data = dirs::data_local_dir().unwrap();
    local_app_data.join("STFC Community Patch")
}

fn get_patch_path(patch_channel: &str) -> Option<String> {
    let patch_file = get_patch_data_path()
        .join("files")
        .join(patch_channel)
        .join("version.dll");
    if patch_file.exists() {
        Some(patch_file.to_str().unwrap().to_string())
    } else {
        None
    }
}

#[tauri::command]
fn launch_game(window: Window) -> bool {
    let game_path = get_game_path();
    if let Some(game_path) = game_path {
        let mut prime_exe = std::path::Path::new(&game_path).join("prime.exe");

        let handle = unsafe {
            let mut prime_exe = prime_exe.as_path().to_str().unwrap().to_string();
            prime_exe.push('\0');
            let mut startup_info = winapi::um::processthreadsapi::STARTUPINFOA::default();
            startup_info.cb =
                std::mem::size_of::<winapi::um::processthreadsapi::STARTUPINFOA>() as u32;

            let mut process_information =
                winapi::um::processthreadsapi::PROCESS_INFORMATION::default();

            let mut game_path = game_path.to_string();
            game_path.push('\0');

            if let Some(mut dll_path) = get_patch_path("live") {
                dll_path.push('\0');
                if ms_detours::DetourCreateProcessWithDllA(
                    std::ptr::null_mut(),
                    prime_exe.as_mut_ptr() as *mut i8,
                    std::ptr::null_mut(),
                    std::ptr::null_mut(),
                    0,
                    0,
                    std::ptr::null_mut(),
                    game_path.as_ptr() as *const i8,
                    &mut startup_info,
                    &mut process_information,
                    dll_path.as_ptr() as *const i8,
                    None,
                ) == 0
                {
                    None
                } else {
                    Some(process_information.hProcess)
                }
            } else {
                if winapi::um::processthreadsapi::CreateProcessA(
                    std::ptr::null_mut(),
                    prime_exe.as_mut_ptr() as *mut i8,
                    std::ptr::null_mut(),
                    std::ptr::null_mut(),
                    0,
                    0,
                    std::ptr::null_mut(),
                    game_path.as_ptr() as *const i8,
                    &mut startup_info,
                    &mut process_information,
                ) == 0
                {
                    None
                } else {
                    Some(process_information.hProcess)
                }
            }
        };

        if let Some(handle) = handle {
            window.emit("game_start", {}).unwrap();
            let handle = HandleWrapper(handle);
            std::thread::spawn(move || {
                wait(handle);
                window.emit("game_closed", {}).unwrap();
            });
            true
        } else {
            window.emit("game_start_failed", {}).unwrap();
            false
        }
    } else {
        false
    }
}

fn get_installed_version() -> i32 {
    let game_path = get_game_path().unwrap();
    if let Ok(version_file) =
        std::fs::read_to_string(std::path::Path::new(&game_path).join(".version"))
    {
        version_file
            .split("=")
            .skip(1)
            .next()
            .map(|v| v.parse::<i32>().ok())
            .flatten()
            .unwrap_or(-1)
    } else {
        -1
    }
}

async fn get_update_xml(version: Option<i32>) -> String {
    let local_version = version.unwrap_or_else(get_installed_version);
    let request_url = format!(
        "https://gus.xsolla.com/updates?version={}&project_id=152033&region=&platform=windows",
        local_version
    );
    let xml = reqwest::get(request_url)
        .await
        .unwrap()
        .text()
        .await
        .unwrap();
    xml
}

#[tauri::command]
async fn check_for_game_updates() -> bool {
    let local_version = get_installed_version();
    let xml = get_update_xml(None).await;

    let doc = roxmltree::Document::parse(&xml).unwrap();
    let elem = doc
        .descendants()
        .find(|n| n.attribute("type") == Some("version"))
        .unwrap();
    let remote_version = elem.attribute("version").unwrap().parse::<i32>().unwrap();
    local_version < remote_version
}

#[derive(Clone, serde::Serialize)]
enum UpdatePayload {
    UpdateStart { extract_count: usize },
    DownloadStart { url: String, size: u64 },
    DownloadProgress { url: String, size: u64 },
    ExtractStart,
    ExtractEnd,
    ApplyStart,
    ApplyEnd,
    UpdateEnd,
}

async fn download_file(
    window: Window,
    client: reqwest::Client,
    url: String,
    path: String,
) -> Result<(), String> {
    use futures_util::stream::StreamExt;
    use std::cmp::min;
    use std::io::Write;

    let res = client
        .get(&url)
        .send()
        .await
        .or(Err(format!("Failed to GET from '{}'", &url)))?;
    let total_size = res
        .content_length()
        .ok_or(format!("Failed to get content length from '{}'", &url))?;

    window
        .emit(
            "update_progress",
            UpdatePayload::DownloadStart {
                url: url.clone(),
                size: total_size,
            },
        )
        .unwrap();

    // download chunks
    let mut file =
        std::fs::File::create(&path).or(Err(format!("Failed to create file '{}'", &path)))?;
    let mut downloaded: u64 = 0;
    let mut stream = res.bytes_stream();

    while let Some(item) = stream.next().await {
        let chunk = item.or(Err(format!("Error while downloading file")))?;
        file.write_all(&chunk)
            .or(Err(format!("Error while writing to file")))?;
        let new = min(downloaded + (chunk.len() as u64), total_size);
        window
            .emit(
                "update_progress",
                UpdatePayload::DownloadProgress {
                    url: url.clone(),
                    size: chunk.len() as u64,
                },
            )
            .unwrap();
        downloaded = new;
    }

    return Ok(());
}

#[derive(serde::Deserialize)]
struct PatchRule {
    file_size: Option<u64>,
    relative_path: String,
    rule: String,
    sha512: Option<String>,
}

fn copy_all(
    src: impl AsRef<std::path::Path>,
    dest: impl AsRef<std::path::Path>,
) -> std::io::Result<()> {
    std::fs::create_dir_all(&dest)?;
    for entry in std::fs::read_dir(src)? {
        let entry = entry?;
        if entry.path().is_dir() {
            copy_all(entry.path(), dest.as_ref().join(entry.file_name()))?;
        } else {
            std::fs::copy(entry.path(), dest.as_ref().join(entry.file_name()))?;
        }
    }

    Ok(())
}

async fn run_update(window: Window, version: i32, mut game_path: String) -> Result<(), String> {
    let xml = get_update_xml(Some(version)).await;
    let doc = roxmltree::Document::parse(&xml)
        .map_err(|e| "Unexpected response from patch server. Unable to parse".to_string())?;

    let client = reqwest::Client::new();

    let temp_dir = tempfile::tempdir().expect("Temp dir creation failed");
    let mut temp_dir = temp_dir
        .path()
        .to_str()
        .expect("Invalid temp dir path")
        .to_string();
    if temp_dir.ends_with("/") || temp_dir.ends_with("\\") {
        let _ = temp_dir.pop();
    }

    // let mut game_path = get_game_path().ok_or("Game path missing".to_string())?;
    if game_path.ends_with("/") || game_path.ends_with("\\") {
        let _ = game_path.pop();
    }
    std::fs::create_dir_all(&temp_dir)
        .map_err(|_| "Creating temp directories failed".to_string())?;

    let temp_target_dir = tempfile::tempdir().expect("Temp dir creation failed");
    let mut temp_target_dir_str = temp_target_dir
        .path()
        .to_str()
        .expect("Invalid temp dir path")
        .to_string();
    if temp_target_dir_str.ends_with("/") || temp_target_dir_str.ends_with("\\") {
        let _ = temp_target_dir_str.pop();
    }

    let mut actions = Vec::new();
    let extract_count = doc
        .descendants()
        .filter(|elem| elem.attribute("type") == Some("extract"))
        .count();

    window
        .emit(
            "update_progress",
            UpdatePayload::UpdateStart { extract_count },
        )
        .unwrap();

    for elem in doc.descendants() {
        if elem.attribute("type") == Some("torrent_download") {
            let url = elem
                .attribute("alt_data_link")
                .ok_or("Missing info in patch info, data_link".to_string())?;
            let path = elem
                .attribute("alt_to")
                .ok_or("Missing info in patch info, to".to_string())?
                .replace("$temp_path", &temp_dir);
            let action =
                download_file(window.clone(), client.clone(), url.to_string(), path).boxed();
            actions.push(action)
        } else if elem.attribute("type") == Some("extract") {
            window
                .emit("update_progress", UpdatePayload::ExtractStart)
                .unwrap();
            let file = elem
                .attribute("file")
                .ok_or("Missing info in patch info, file".to_string())?
                .replace("$temp_path", &temp_dir)
                .replace("$game_path", &temp_target_dir_str);
            let path = elem
                .attribute("to")
                .ok_or("Missing info in patch info, to".to_string())?
                .replace("$temp_path", &temp_dir)
                .replace("$game_path", &temp_target_dir_str);

            sevenz_rust::decompress_file(file, path).expect("complete");

            window
                .emit("update_progress", UpdatePayload::ExtractEnd)
                .unwrap();
        } else if elem.attribute("type") == Some("patch") {
            let binaries = elem
                .attribute("binaries")
                .ok_or("Missing info in patch info, binaries".to_string())?
                .replace("$temp_path", &temp_dir)
                .replace("$game_path", &temp_target_dir_str);
            let patch = elem
                .attribute("patch")
                .ok_or("Missing info in patch info, patch".to_string())?
                .replace("$temp_path", &temp_dir);

            let patch_rules: Vec<PatchRule> = serde_json::from_str(
                &std::fs::read_to_string(std::path::Path::new(&patch).join("patchRules.json"))
                    .map_err(|_| "Failed to read patchRules.json".to_string())?,
            )
            .map_err(|_| "Failed to parse patchRules.json".to_string())?;

            for rule in patch_rules {
                let mut relative_path = rule.relative_path;
                if relative_path.starts_with("/") || relative_path.starts_with("\\") {
                    relative_path = relative_path[1..].to_string();
                }

                let target_path = std::path::Path::new(temp_target_dir.path()).join(&relative_path);
                let _ = std::fs::create_dir_all(target_path.parent().expect("Parent missing"));
                let source_path = std::path::Path::new(&game_path).join(&relative_path);
                let patch_patch = std::path::Path::new(&patch).join(&relative_path);

                if rule.rule == "patch" {
                    if !target_path.exists() {
                        std::fs::copy(&source_path, &target_path).map_err(|_| {
                            format!(
                                "Failed to copy file from {} to {}",
                                source_path.display(),
                                target_path.display()
                            )
                        })?;
                    }
                    let source = std::fs::read(&target_path)
                        .map_err(|_| format!("Failed to read source: {}", target_path.display()))?;
                    let patch = std::fs::read(&patch_patch)
                        .map_err(|_| format!("Failed to read patch: {}", target_path.display()))?;

                    let mut out = std::fs::File::create(&target_path).map_err(|_| {
                        format!("Failed to create target: {}", target_path.display())
                    })?;
                    if let Err(e) = fast_rsync::apply(&source, &patch, &mut out) {
                        println!("{:?} {:?}: {}", target_path, patch_patch, e);
                    }
                } else if rule.rule == "create" {
                    let _ = std::fs::create_dir_all(target_path.parent().expect("Parent missing"));
                    let _ = std::fs::File::create(&target_path);
                } else if rule.rule == "delete" {
                    let _ = std::fs::remove_file(&target_path);
                } else if rule.rule == "copy" {
                    let _ = std::fs::copy(patch_patch, target_path);
                }
            }
        } else if elem.attribute("type") == Some("wait_actions") {
            join_all(actions).await;
            actions = Vec::new();
        } else if elem.attribute("type") == Some("extracted_size") {
            // Don't give a shit
        } else if elem.attribute("type") == Some("version") {
            use std::io::Write;
            let target_path = std::path::Path::new(temp_target_dir.path()).join(".version");
            let mut version_file = std::fs::File::create(target_path)
                .map_err(|_| "Failed to open .version file".to_string())?;
            let _ = version_file
                .write_all(format!("&game={}", elem.attribute("version").unwrap()).as_bytes());
        } else if elem.attribute("type").is_some() {
            println!("{:?}", elem.attribute("type"));
            unreachable!();
        }
    }

    window
        .emit("update_progress", UpdatePayload::ApplyStart)
        .unwrap();
    let game_path = std::path::Path::new(&game_path);
    // copy_all(temp_target_dir.path(), &game_path).unwrap();
    window
        .emit("update_progress", UpdatePayload::ApplyEnd)
        .unwrap();

    window
        .emit("update_progress", UpdatePayload::UpdateEnd)
        .unwrap();

    Ok(())
}

#[tauri::command]
async fn apply_update(window: Window) -> Result<(), String> {
    let version = get_installed_version();
    let game_path = get_game_path().ok_or("Game Path missing".to_string())?;
    run_update(window, version, game_path).await
}

#[tauri::command]
async fn install_game(window: Window, path: String) -> Result<(), String> {
    run_update(window, -1, path).await
}

#[tauri::command]
async fn patch_installed(window: Window, channel: String) -> bool {
    let patch_data_path = get_patch_data_path();
    patch_data_path
        .join("files")
        .join(channel)
        .join("version.dll")
        .exists()
}

#[tauri::command]
async fn update_patch(window: Window, channel: String) -> bool {
    let client = reqwest::Client::new();
    let patch_data_path = get_patch_data_path();
    let version_dll_file = patch_data_path
        .join("files")
        .join(channel)
        .join("version.dll");

    let _ = std::fs::create_dir_all(version_dll_file.parent().unwrap());

    download_file(
        window,
        client,
        "https://assets.stfc.lol/mods/community-patch/0.5.3/version.dll".to_owned(),
        version_dll_file.to_str().unwrap().to_string(),
    )
    .await
    .is_ok()
}

mod github;

#[tauri::command]
async fn check_patch_update(window: Window, channel: String) -> bool {
    let latest_release = reqwest::Client::builder()
        .user_agent("Testing")
        .build()
        .unwrap()
        .get("https://api.github.com/repos/tashcan/bob/releases/latest")
        .send()
        .await
        .unwrap()
        .text()
        .await
        .unwrap();
    let latest_release = serde_json::from_str::<github::GithubRelease>(&latest_release).unwrap();

    let version = &latest_release.tag_name[1..];

    if let Some(patch_path) = get_patch_path(&channel) {
        let bytes = std::fs::read(patch_path).unwrap();
        use pelite::pe64;
        let resources = match pe64::PeFile::from_bytes(&bytes) {
            Ok(file) => {
                use pelite::pe64::Pe;

                file.resources()
            }
            Err(pelite::Error::PeMagic) => {
                use pelite::pe32::{Pe, PeFile};

                PeFile::from_bytes(&bytes).unwrap().resources()
            }
            Err(e) => Err(e),
        }
        .unwrap();

        let version_info = resources.version_info().unwrap();
        let on_disk_version = version_info
            .translation()
            .first()
            .and_then(|language| version_info.value(*language, "ProductVersion"))
            .unwrap();

        use version_compare::{compare, Cmp, Version};

        println!("{} {}", version, on_disk_version);
        compare(on_disk_version, version) == Ok(Cmp::Lt)
    } else {
        false
    }
}

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![
            get_game_path,
            launch_game,
            check_for_game_updates,
            apply_update,
            game_installed,
            install_game,
            update_patch,
            patch_installed,
            check_patch_update,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
