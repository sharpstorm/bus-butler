import { Api } from './api'
import { SetupState } from './setup-state'
import { setAppState } from './state'

let setupState: SetupState | undefined

// Theme management
function initTheme() {
  const themeToggle = document.getElementById('theme-toggle')
  const savedTheme = localStorage.getItem('theme')
  const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches

  // Set initial theme
  if (savedTheme) {
    document.documentElement.setAttribute('data-theme', savedTheme)
  } else if (prefersDark) {
    document.documentElement.setAttribute('data-theme', 'dark')
  } else {
    document.documentElement.setAttribute('data-theme', 'light')
  }

  // Theme toggle handler
  themeToggle?.addEventListener('click', () => {
    const currentTheme = document.documentElement.getAttribute('data-theme')
    const newTheme = currentTheme === 'dark' ? 'light' : 'dark'
    document.documentElement.setAttribute('data-theme', newTheme)
    localStorage.setItem('theme', newTheme)
  })
}

document.addEventListener('DOMContentLoaded', async () => {
  initTheme()

  const configuredSsid = await Api.getConfiguredWifiSsid()
  if (configuredSsid) {
    setAppState('config')
  } else {
    setupState = new SetupState()
    setupState.onWorkflowComplete(() => {
      setAppState('config')
    })
    setAppState('setup')
  }
})
