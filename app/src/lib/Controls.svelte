<script lang="ts">
    import { invoke } from "@tauri-apps/api/tauri";
    import { open } from "@tauri-apps/api/dialog";
    import { emit, listen } from "@tauri-apps/api/event";
    import { onDestroy } from "svelte";

    const launchPhrases = ["Engage!", "Make It So!", "Hit It!"];
    let launchPhrase =
        launchPhrases[Math.floor(Math.random() * launchPhrases.length)];

    let totalDownloaded = 0;
    let totalUpdateSize = 0;
    let extractCount = 0;
    let extractStart = 0;
    let extractDone = 0;
    let updateProgress = 0;

    let patchAction = "Idle";
    listen("update_progress", ({ payload }) => {
        if (payload.UpdateStart) {
            patchAction = "Preparing";
            totalDownloaded = 0;
            totalUpdateSize = 0;
            extractCount = 0;
            extractStart = 0;
            extractDone = 0;
            extractCount = payload.UpdateStart.extract_count;

            updateProgress = 0;
        } else if (payload.DownloadStart) {
            patchAction = "Download";
            totalUpdateSize += payload.DownloadStart.size;
        } else if (payload.DownloadProgress) {
            totalDownloaded += payload.DownloadProgress.size;
            updateProgress = totalDownloaded / totalUpdateSize;
        } else if (payload === "ExtractStart") {
            patchAction = "Extract";
            updateProgress = extractDone / extractCount;
            extractStart++;
        } else if (payload === "ExtractEnd") {
            extractDone++;
            updateProgress = extractDone / extractCount;
        } else if (payload === "ApplyStart") {
            patchAction = "Apply";
        } else if (payload === "ApplyEnd") {
            patchAction = "Idle";
        }
    });

    let isPatchInstalled = false;
    let isGameInstalled = false;
    const checkGameInstalled = async () => {
        isGameInstalled = await invoke("game_installed");
    };
    checkGameInstalled();
    const checkPatchInstalled = async () => {
        isPatchInstalled = await invoke("patch_installed", { channel: "live" });
    };
    checkPatchInstalled();

    let gameUpdateAvailable = false;
    const checkForGameUpdates = () => {
        invoke("check_for_game_updates").then((hasUpdate: boolean) => {
            gameUpdateAvailable = hasUpdate;
        });
    };
    checkForGameUpdates();

    let patchUpdateAvailable = false;
    const checkForPatchUpdates = () => {
        invoke("check_patch_update", { channel: "live" }).then(
            (hasUpdate: boolean) => {
                patchUpdateAvailable = hasUpdate;
            }
        );
    };
    checkForPatchUpdates();

    let gameRunning = false;
    listen("game_start", () => {
        gameRunning = true;
    });
    listen("game_closed", () => {
        gameRunning = false;
        checkForGameUpdates();
    });

    const launchGame = async () => {
        await invoke("launch_game");
    };

    let updateInProgress = false;
    const updateGame = async () => {
        patchAction = "Preparing";
        updateInProgress = true;

        totalDownloaded = 0;
        totalUpdateSize = 0;
        updateProgress = 0;

        await invoke("apply_update");

        updateInProgress = false;
        checkForGameUpdates();
    };

    const installGame = async () => {
        const gamePath = await open({
            directory: true,
        });
        await invoke("install_game", { path: gamePath });
    };

    const updatePatch = async () => {
        let channel = "live";
        await invoke("update_patch", { channel });
        await checkForPatchUpdates();
        await checkPatchInstalled();
    };
</script>

<div class="controls">
    <div class="buttons row">
        <button
            class="launch-button"
            on:click={launchGame}
            disabled={gameRunning}
            >{gameRunning ? "Running" : launchPhrase}</button
        >
        {#if gameUpdateAvailable}
            <button
                disabled={updateInProgress || gameRunning}
                on:click={updateGame}>Update Game</button
            >
        {:else}
            <button on:click={checkForGameUpdates}>Check Game Updates</button>
        {/if}
        {#if patchUpdateAvailable || !isPatchInstalled}
            <button on:click={updatePatch}
                >{isPatchInstalled
                    ? "Update Community Patch"
                    : "Install Community Patch"}</button
            >
        {:else}
            <button on:click={checkForPatchUpdates}
                >Check Patch Updates</button
            >
        {/if}
    </div>
    {#if updateInProgress}
        <div class="action-progess">
            <div class="flex">
                {#if patchAction == "Preparing"}
                    Preparing
                {:else if patchAction == "Download"}
                    Downloading {Math.round(totalDownloaded / 1000 / 1000)} MB of
                    {Math.round(totalUpdateSize / 1000 / 1000)} MB
                {:else if patchAction == "Extract"}
                    Extracting {extractStart} of {extractCount}
                {:else if patchAction == "Apply"}
                    Applying updates
                {:else if patchAction == "Idle"}
                    Done
                {/if}
            </div>
            <div style="width: 100%; background-color: white;">
                <div
                    class="progress-bar"
                    style="width: {updateProgress * 100}%"
                />
            </div>
        </div>
    {/if}
</div>

<style>
    .progress-bar {
        height: 1rem;
        background-color: green;
    }
    .buttons {
        margin-top: auto;
        margin-bottom: 1rem;
        margin-left: 1rem;
        height: min-content;

        display: flex;
        gap: 1rem;

        align-items: center;
        height: 100%;
    }
    .buttons button {
        height: 5rem;
    }
    .controls {
        display: grid;
        grid-template-columns: 1fr 4fr;
        justify-content: space-between;
        gap: 1rem;
    }
    .action-progess {
        display: flex;
        flex-direction: column;
        gap: 0.25rem;
        margin: 1rem;
        align-items: flex-start;
    }
    .launch-button {
        width: 6.25rem;
        background-color: rgba(0, 255, 0, 0.6);
        color: black;
    }

    .flex {
        display: flex;
    }
</style>
