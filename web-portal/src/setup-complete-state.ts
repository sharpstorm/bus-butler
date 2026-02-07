import { setAppState } from './state'
import { dashboardState } from './state-store'
import { findAndAssertDomId } from './utils'

export function initSetupCompleteState() {
  const dashboardBtn = findAndAssertDomId('open-dashboard-btn')
  dashboardBtn.addEventListener('click', () => {
    setAppState('dashboard')
    dashboardState.initSettings()
  })
}
