import { defineConfig, presetAttributify, presetUno } from 'unocss'
import presetWind from '@unocss/preset-wind';

export default defineConfig({
    content: {
        pipeline: {
          include: [
            // the default
            /\.(vue|svelte|[jt]sx|mdx?|astro|elm|php|phtml|html)($|\?)/,
            // include js/ts files
            'src/**/*.{js,ts}',
            'node_modules/@tashcan/**/*.svelte',
          ],
          // exclude files
          // exclude: []
        }
    },
presets: [
    presetAttributify({ /* preset options */}),
    presetUno(),
    presetWind(),
    {
        name: 'my-preset',
        shortcuts: {
          'input-bg-color': 'bg-dark-50 dark:bg-dark-600 dark:text-dark-50',
          'input-bg': 'dark:border-transparent border bg-light-400 dark:bg-dark-200 dark:text-light-50',
          'input-focus':
            'focus:outline-none focus:ring-1 dark:focus:ring-1 focus:ring-indigo-500 focus:border-indigo-500 dark:focus:border-indigo-500',
          'detail-page-header':
            'bg-light-400 dark:bg-dark-400 dark:text-light-300 w-full shadow relative p-2 px-2 sm:px-4'
        }
        // ...
    }
]
})
