export type AppState = 'setup' | 'config' | 'loading'

const stateToContainerIdMap: Record<AppState, string> = {
  loading: 'state-loading',
  setup: 'state-setup',
  config: 'state-config',
}

export type AppStateStore = {
  state: AppState
}

export const appState: AppStateStore = {
  state: 'loading',
}

export const setAppState = (newState: AppState) => {
  appState.state = newState

  Object.entries(stateToContainerIdMap).forEach(([k, v]) => {
    if (k === newState) {
      document.getElementById(v)?.classList.remove('hidden')
    } else {
      document.getElementById(v)?.classList.add('hidden')
    }
  })
}
