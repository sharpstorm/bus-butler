import { defineConfig } from 'vite';
import { createMockApiMiddleware } from './mock/mock-api'

export default defineConfig({
  base: './',
  build: {
    assetsDir: '.',
    rollupOptions: {
      output: {
        entryFileNames: `[name].js`,
        chunkFileNames: `[name].js`,
        assetFileNames: `[name].[ext]`,
      },
      external: ['mock/**s']
    },
  },
  plugins: [
    {
      name: 'mock-api-server',
      configureServer(server) {
        // Add mock API middleware to Vite's dev server
        server.middlewares.use(createMockApiMiddleware())
      },
    },
  ],
});
