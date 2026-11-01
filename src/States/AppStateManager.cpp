#include "AppStateManager.h"

#include "../Ui/Anim.h"
#include "Controls/ControlsState.h"

const AppState AppStateManager::PAGES[] = {AppState::Home, AppState::WeatherPage,
                                           AppState::MrtPage,
                                           AppState::StatusPage};
const int AppStateManager::PAGE_COUNT = 4;

IAppState* AppStateManager::getActiveState() { return activeStateRef; }

AppState AppStateManager::getActiveStateId() { return activeState; }

void AppStateManager::changeState(AppState state) {
  if (activeStateRef != nullptr) {
    activeStateRef->exit();
  }

  activeState = state;
  activeStateRef = stateResolver->getStateRefById(state);

  if (activeStateRef != nullptr) {
    activeStateRef->enter();
  }
}

int AppStateManager::pageIndex(AppState state) {
  for (int i = 0; i < PAGE_COUNT; i++) {
    if (PAGES[i] == state) return i;
  }
  return -1;
}

bool AppStateManager::isOverlay(AppState state) {
  return state == AppState::Controls || state == AppState::StopQr ||
         state == AppState::Notify;
}

void AppStateManager::openOverlay(AppState overlay) {
  if (activeState == overlay) return;
  if (!isOverlay(activeState)) {
    returnState = activeState;
  }
  anim::capture();
  changeState(overlay);
  if (overlay == AppState::Controls) {
    anim::panelDown(CONTROLS_H);
  } else {
    anim::slide(anim::Slide::Up);
  }
}

void AppStateManager::closeOverlay() {
  AppState closing = activeState;
  anim::capture();
  changeState(returnState);
  if (closing == AppState::Controls) {
    anim::panelUp(CONTROLS_H);
  } else {
    anim::slide(anim::Slide::Down);
  }
}

void AppStateManager::goToPage(AppState page) {
  int from = pageIndex(activeState);
  int to = pageIndex(page);
  anim::capture();
  changeState(page);
  anim::slide(to >= from ? anim::Slide::Left : anim::Slide::Right);
}

void AppStateManager::redraw() {
  if (activeStateRef != nullptr) {
    activeStateRef->exit();
    activeStateRef->enter();
  }
}

void AppStateManager::dispatchGesture(const Gesture& gesture) {
  IAppState* state = activeStateRef;
  if (state == nullptr) return;

  if (gesture.type == GestureType::SwipeDown &&
      gesture.startY < TOP_EDGE_ZONE && state->allowsControls()) {
    openOverlay(AppState::Controls);
    return;
  }

  if (state->onGesture(gesture)) return;

  if (state->isPage() && gesture.isHorizontal()) {
    int index = pageIndex(activeState);
    if (index < 0) return;
    int next = gesture.type == GestureType::SwipeLeft ? index + 1 : index - 1;
    if (next >= 0 && next < PAGE_COUNT) {
      goToPage(PAGES[next]);
    }
  }
}

void AppStateManager::registerAppStateResolver(IAppStateResolver* resolver) {
  stateResolver = resolver;
}

AppStateManager appStateManager;
