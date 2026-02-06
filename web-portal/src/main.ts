import { Api } from './api'
import { SetupState } from './setup-state'
import { DashboardState } from './dashboard-state'
import { setAppState } from './state'
import { findAndAssertDomId } from './utils'

let setupState: SetupState | undefined
let dashboardState: DashboardState | undefined

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

function initDashboardButton() {
  const dashboardBtn = findAndAssertDomId('open-dashboard-btn')
  dashboardBtn.addEventListener('click', () => {
    setAppState('dashboard')
    if (!dashboardState) {
      dashboardState = new DashboardState()
    }
  })
}

document.addEventListener('DOMContentLoaded', async () => {
  initTheme()
  initDashboardButton()

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
