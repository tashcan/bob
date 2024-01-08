<script lang="ts">
  import Controls from "./lib/Controls.svelte";

  import "virtual:uno.css";
  import "@unocss/reset/tailwind-compat.css";
</script>

<svelte:head>
  <title>STFC Community Patch</title>
</svelte:head>

<main class="text-zinc-200">
  <div class="main-grid">
    <div class="content-grid">
      <div class="p-2 flex flex-col">
        <div>
          <label for="release-channel">Release Channel:</label>
          <select id="release-channel">
            <option>Stable</option>
            <option>Pre-Release</option>
          </select>
        </div>
        <div>
          <input type="checkbox" />
          <label>Hotkeys Enabled</label>
        </div>
      </div>
      <div>News</div>
    </div>
    <Controls />
  </div>
</main>

<style>
  main {
    height: 100vh;
  }

  .content-grid {
    display: grid;
    grid-template-columns: 1.5fr 1fr;
    gap: 0 1rem;
    margin: 1rem;
  }
  .content-grid > div {
    background-color: #0f0f0f98;
    box-shadow: 0 10px 15px -3px rgb(0 0 0 / 0.1),
      0 4px 6px -4px rgb(0 0 0 / 0.1);
  }

  select {
    background: #000;
    padding: 0.25rem;
  }
  select > option {
    padding: 0.25rem;
  }
  select:focus {
    outline: none;
  }
</style>
