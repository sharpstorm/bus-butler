import { Api } from './api'
import { initSetupCompleteState } from './setup-complete-state'
import { setAppState } from './state'
import { dashboardState, setupState } from './state-store'
import { findAndAssertDomId } from './utils'

// Theme management
function initTheme() {
  const themeToggle = findAndAssertDomId('theme-toggle')
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
  themeToggle.addEventListener('click', () => {
    const currentTheme = document.documentElement.getAttribute('data-theme')
    const newTheme = currentTheme === 'dark' ? 'light' : 'dark'
    document.documentElement.setAttribute('data-theme', newTheme)
    localStorage.setItem('theme', newTheme)
  })
}

document.addEventListener('DOMContentLoaded', async () => {
  initTheme()
  initSetupCompleteState()

  const { ssid } = await Api.getConfiguredWifiSsid()
  const { isApiKeySaved } = await Api.getSavedLtaApiKey()
  if (ssid && isApiKeySaved) {
    setAppState('dashboard')
    dashboardState.initSettings()
    return
  }

  if (ssid) {
    setupState.skipWifiSetup()
  }
  setupState.onWorkflowComplete(() => {
    setAppState('setup-complete')
  })
  setAppState('setup')
})
