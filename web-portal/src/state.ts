export const APP_STATES = [
  'loading',
  'setup',
  'setup-complete',
  'dashboard',
] as const
export type AppState = (typeof APP_STATES)[number]

export type AppStateStore = {
  state: AppState
}

export const appState: AppStateStore = {
  state: 'loading',
}

export const setAppState = (newState: AppState) => {
  appState.state = newState

  APP_STATES.forEach((state) => {
    const element = document.getElementById(`state-${state}`)
    if (element) {
      element.classList.toggle('hidden', state !== newState)
    }
  })
}
