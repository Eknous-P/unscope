/*
unscope - an audio oscilloscope
Copyright (C) 2025-2026 Eknous

unscope is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 2 of the License, or (at your option) any later
version.

unscope is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
unscope. If not, see <https://www.gnu.org/licenses/>. 
*/

#include "gui.h"
#include "imgui.h"
#include "font_fontaudio.h"
#include "IconsFontaudio.h"

bool USCGUI::isRunning() {
  return running;
}

USCGUI::USCGUI(USCData* _data, USCConfig* conf, string layoutFilePath) {
  data = _data;
  config = conf;

  layoutFile = layoutFilePath;

#define _S(...) SettingsCategory(__VA_ARGS__)
#define _P(...) Parameter(__VA_ARGS__)

  settingsParams = SettingsCategory(NULL, {}, {
    _S("general", {
      _P(PARAM_TOGGLE, false, "dummy", "dummy", NULL, NULL, &settings.dummy),
      _P(PARAM_TOGGLE, false, "enableMultiViewports", "enable multi viewport", NULL, NULL, &settings.enableMultiViewports),
      _P(PARAM_TOGGLE, false, "scopeContainChannelControls", "put channel controls inside scope window", NULL, NULL, &settings.scopeContainChannelControls),
      _P(PARAM_TOGGLE, false, "showDisabledBuffers", "show uninitialized buffers when selecting a buffer", NULL, NULL, &settings.showDisabledBuffers),
    }, {}),
    _S("colors", {}, {
      _S("generic", {
        _P(PARAM_COLOR, false, "widgetActiveColor", "widget active color", NULL, NULL, &colors.widgetActiveColor),
        _P(PARAM_COLOR, false, "defaultChanColor", "default channel color", NULL, NULL, &colors.defaultChannelColor),
      }, {}),
      _S("piano view", {
        _P(PARAM_COLOR, false, "pianoTopColor", "top key color", NULL, NULL, &colors.pianoTopColor),
        _P(PARAM_COLOR, false, "pianoBottomColor", "bottom key color", NULL, NULL, &colors.pianoBottomColor),
      }, {}),
    }),
  });

#undef _S
#undef _P

  // generateFFTFrequencies();

  wo.aboutOpen           = false;
  wo.cursorsOpen         = true;
  wo.dataIOConfigOpen     = false;
  wo.settingsOpen        = false;
  wo.spectrumOpen        = true;
  wo.spectrumControlsOpen= true;
#ifdef PROGRAM_DEBUG
  wo.metricsOpen         = false;
  wo.paramDebugOpen      = false;
  wo.triggerDebugOpen    = false;
  wo.fftDebugOpen        = false;
#endif
  running = false;

  doFallback = true;
  singleShot = false;

  fullscreen = false;

  errorText="";

  pianoColors[0]  = &colors.pianoBottomColor;
  pianoColors[1]  = &colors.pianoTopColor;
  pianoColors[2]  = &colors.pianoBottomColor;
  pianoColors[3]  = &colors.pianoTopColor;
  pianoColors[4]  = &colors.pianoBottomColor;
  pianoColors[5]  = &colors.pianoBottomColor;
  pianoColors[6]  = &colors.pianoTopColor;
  pianoColors[7]  = &colors.pianoBottomColor;
  pianoColors[8]  = &colors.pianoTopColor;
  pianoColors[9]  = &colors.pianoBottomColor;
  pianoColors[10] = &colors.pianoTopColor;
  pianoColors[11] = &colors.pianoBottomColor;

  rd = NULL;

  newScopeBuffer = NULL;
  newDriver = DATA_PORTAUDIO;
  newScopeWin = 0;

#ifdef PROGRAM_DEBUG
  debuggedWindow=0;
  debuggedChannel=0;
#endif
}

void USCGUI::setupRenderer(USCRenderers r) {
  switch (r) {
#ifdef USE_OPENGL
    case USC_RENDER_OPENGL2:
      rd = new USCRenderOpenGL2;
      break;
#endif
#ifdef USE_DIRECTX9
    case USC_RENDER_DIRECTX9:
        rd = new USCRenderDirectX9;
        break;
#endif
#ifdef USE_DIRECTX11
    case USC_RENDER_DIRECTX11:
        rd = new USCRenderDirectX11;
        break;
#endif
    case USC_RENDER_SDLRENDERER2:
      rd = new USCRenderSDLRenderer;
      break;
    case USC_RENDER_NONE:
    default: break;
  }
}

int USCGUI::init(USCRenderers rend) {
  if (running) {
    printf(MISC_MSG "GUI: not initialzing while running!" MSG_END);
    return 0;
  }
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS)!=0) {
    printf(ERROR_MSG "GUI: failed to initialze SDL!" MSG_END);
    printf(INFO_MSG "SDL error: %s" MSG_END, SDL_GetError());
    return -1;
  }
  renderer = rend;
  setupRenderer(renderer);
  if (rd->initRender()!=0) {
    printf(ERROR_MSG "GUI: file to initialize renderer!" MSG_END);
    return -1;
  }
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::GetIO().IniFilename = NULL;
  if (settings.enableMultiViewports)
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::GetIO().Fonts->AddFontDefaultVector();
  ImFontConfig fontConfig;
  fontConfig.MergeMode = true;
  fontConfig.GlyphMinAdvanceX = 16.0f;
  fontConfig.FontDataOwnedByAtlas = false;
  fontConfig.GlyphOffset={0,4};
  ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)font_fontaudio_ttf, font_fontaudio_ttf_len, 16.0f, &fontConfig);

  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
  if (layoutFile.empty()) {
    ImGui::LoadIniSettingsFromMemory(windowLayout);
  } else {
    ImGui::LoadIniSettingsFromDisk(layoutFile.c_str());
  }
  if (rd->setupRender(
    (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI),
    PROGRAM_NAME_AND_VER,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    windowWidth, windowHeight)!=0) return -1;
  bgColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  doFullscreen();
  running = true;

  printf(SUCCESS_MSG "GUI: initialized successfully." MSG_END);
  return 0;
}

void USCGUI::doFrame() {
  int s;
  s=rd->renderPreLoop();
  switch (s) {
    case 1:
      if (!(ImGui::GetIO().ConfigFlags&ImGuiConfigFlags_ViewportsEnable)) SDL_Delay(100); // replace with viewports setting later
      break;
    case 0: break;
    default:
      running=false;
      break;
  }
  if (!running) return;
  ImGui::NewFrame();
  
  ImGui::DockSpaceOverViewport(ImGui::GetID("dock"),ImGui::GetMainViewport());

  drawGUI();

  ImGui::Render();
  running &= rd->renderPostLoop()>=0;
}

void USCGUI::doFullscreen() {
  SDL_SetWindowFullscreen(rd->getWindow(),fullscreen?(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP):0);
}

void USCGUI::drawGUI() {
  if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
    fullscreen = !fullscreen;
    doFullscreen();
  }
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("Data I/O Configuration", NULL, &wo.dataIOConfigOpen);
      ImGui::MenuItem("Settings", NULL, &wo.settingsOpen);
      ImGui::Separator();
      if (ImGui::MenuItem("Fullscreen", "F11", fullscreen)) {
        fullscreen = !fullscreen;
        doFullscreen();
      }
      if (ImGui::MenuItem("Quit")) running = false;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Scopes")) {
      char strbuf[64];
      const int scopeWins = scopeWindows.size();
      for (int i=0; i<scopeWins; i++) {
        snprintf(strbuf, 64, "Scope Window %d", i+1);
        ImGui::MenuItem(strbuf, NULL, &scopeWindows[i].windowOpen);
      }
      if (ImGui::MenuItem("New Scope Window...")) {
        scopeWindows.push_back(ScopeWindow(data,scopeWins));
      }
      ImGui::EndMenu();
    }
#ifdef PROGRAM_DEBUG
    if (ImGui::BeginMenu("Debug")) {
      ImGui::MenuItem("Metrics",NULL,&wo.metricsOpen);
      ImGui::Separator();
      ImGui::MenuItem("Trigger...",NULL,&wo.triggerDebugOpen);
      ImGui::MenuItem("Parameters...",NULL,&wo.paramDebugOpen);
      ImGui::EndMenu();
    }
#endif
    if (ImGui::BeginMenu("About")) {
      ImGui::MenuItem("About...",NULL,&wo.aboutOpen);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  drawSettings(&wo.settingsOpen);
  drawAbout(&wo.aboutOpen);
  drawDataConfig(&wo.dataIOConfigOpen);
  drawChanControls();

  for (ScopeWindow& s:scopeWindows) s.drawScopeWindow(settings.scopeContainChannelControls, settings.showDisabledBuffers);

#ifdef PROGRAM_DEBUG
  if (wo.metricsOpen) ImGui::ShowMetricsWindow(&wo.metricsOpen);
  drawTriggerDebug(&wo.triggerDebugOpen);
  drawParamDebug(&wo.paramDebugOpen);
#endif
  if (ImGui::BeginPopupModal("Error##ERRPOPUP", NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize)) {
    ImGui::Text("%s", errorText.c_str());
    if (ImGui::Button("OK")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

#ifdef PROGRAM_DEBUG
void USCGUI::drawTriggerDebug(bool* open) {
  if (!*open) return;
  constexpr int DIV=16;
  if (ImGui::Begin("Trigger Debug", open)) {
    bool canDraw=true;
    if (scopeWindows.empty()) {
      canDraw = false;
      ImGui::Text("no windows available...");
    } else {
      if (ImGui::InputInt("window", &debuggedWindow)) {
        if (debuggedWindow<0) debuggedWindow=0;
        if (debuggedWindow>scopeWindows.size()-1) debuggedWindow=scopeWindows.size()-1;
      }
      if (scopeWindows[debuggedWindow].channels.empty()) {
        canDraw = false;
        ImGui::Text("selected window has no channels...");
      } else {
        if (ImGui::InputInt("channel", &debuggedChannel)) {
          if (debuggedChannel<0) debuggedChannel=0;
          if (debuggedChannel>scopeWindows[debuggedWindow].channels.size()-1)
            debuggedChannel=scopeWindows[debuggedWindow].channels.size()-1;
        }
      }
    }
    if (canDraw) {
      ImVec2 origin = ImGui::GetCursorScreenPos();
      ImRect rect(origin, origin+ImGui::GetContentRegionAvail());
      ImDrawList* dl = ImGui::GetWindowDrawList();
      ScopeChannel* chan = scopeWindows[debuggedWindow].channels.at(debuggedChannel);
      const float* bufferData = (float*)chan->getBuffer()->getBuffer();
      const nint bufferSize = chan->getBuffer()->getSize()/DIV;
      ImVec2* plot = new ImVec2[bufferSize];

      for (nint i=0; i<bufferSize; i++) {
        plot[i] = ImVec2(
          ImLerp(rect.Min.x, rect.Max.x, (float)i/bufferSize),
          ImLerp(rect.GetCenter().y, rect.Min.y, bufferData[i*DIV])
        );
      }

      dl->AddPolyline(plot, bufferSize, 0xffffffff, 0, 1.0f);

      Trigger* trig = chan->getTrigger();
      ImGui::Text("trigger: %p", trig);
      if (trig) {
        nint needlePos = (chan->getNeedle()+chan->getBuffer()->getIndex())%chan->getBuffer()->getSize();
        ImGui::Text("needle: %llu (%llu)", needlePos, trig->getTriggerIndex());
        float trigNeedle = (float)needlePos/chan->getBuffer()->getSize();
        trigNeedle = ImLerp(rect.Min.x, rect.Max.x, trigNeedle);
        dl->AddLine(
          ImVec2(trigNeedle,rect.Min.y),
          ImVec2(trigNeedle,rect.Max.y),
          0xff00ff00, 2.0f
        );
      }

      delete[] plot;
    }
  }
  ImGui::End();
}

void USCGUI::drawParamDebug(bool* open) {
  if (!*open) return;
  if (ImGui::Begin("Parameter Debug", open)) {
    if (ImGui::BeginTable("params", 8, ImGuiTableFlags_Resizable)) {
      ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
      ImGui::TableNextColumn();
      ImGui::Text("from");
      ImGui::TableNextColumn();
      ImGui::Text("label");
      ImGui::TableNextColumn();
      ImGui::Text("pointer");
      ImGui::TableNextColumn();
      ImGui::Text("type");
      ImGui::TableNextColumn();
      ImGui::Text("value");
      ImGui::TableNextColumn();
      ImGui::Text("width");
      ImGui::TableNextColumn();
      ImGui::Text("hovered");
      ImGui::TableNextColumn();
      ImGui::Text("active");
      vector<std::pair<const char*, Parameter*>> allParams;
      for (int i=0; i<data->getDriverCount(); i++) {
        for (int j=0; j<data->getDriver(i)->getParams()->size(); j++)
          allParams.push_back({
            "data driver",
            &data->getDriver(i)->getParams()->at(j)
          });
      }
      for (int i=0; i<scopeWindows.size(); i++) {
        for (int j=0; j<scopeWindows[i].channels.size(); j++) {
          for (int k=0; k<scopeWindows[i].channels[j]->getParams()->size(); k++) {
            allParams.push_back({
              "scope channel",
              &scopeWindows[i].channels[j]->getParams()->at(k)
            });
          }
          if (scopeWindows[i].channels[j]->getTrigger()!=NULL) {
            for (int k=0; k<scopeWindows[i].channels[j]->getTrigger()->getParams()->size(); k++) {
              allParams.push_back({
                "scope channel trigger",
                &scopeWindows[i].channels[j]->getTrigger()->getParams()->at(k)
              });
            }
          }
        }
      }
      for (auto i:allParams) {
        Parameter* p=i.second;
        ImGui::PushID(p);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(i.first);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(p->getLabel());
        ImGui::TableNextColumn();
        ImGui::Text("%p", p);
        ImGui::TableNextColumn();
        ImGui::Text("%s", paramTypeNames[p->getType()]);
        ImGui::TableNextColumn();
        switch (p->getType()) {
          case PARAM_TOGGLE:
          case PARAM_TEXTTOGGLE:
            ImGui::Checkbox("##toggle", (bool*)p->getValuePtr());
            break;
          case PARAM_KNOBFLOAT:
          case PARAM_KNOBNORM:
          case PARAM_KNOBUNIT:
          case PARAM_INPUTFLOAT:
            ImGui::InputFloat("##input", (float*)p->getValuePtr());
            break;
          case PARAM_INPUTINT:
          case PARAM_COMBO_CSTR:
          case PARAM_COMBO_INT:
          case PARAM_COMBOV_STR:
            ImGui::InputInt("##input", (int*)p->getValuePtr());
            break;
          case PARAM_COLOR:
            ImGui::ColorButton("##color", ImGui::ColorConvertU32ToFloat4(p->getValue<ImU32>()));
            break;
          default: break;
        }
        ImGui::TableNextColumn();
        ImGui::Text("%f", p->getEstimatedWidth());
        ImGui::TableNextColumn();
        ImGui::Text("%s", p->isHovered()?"true":"false");
        ImGui::TableNextColumn();
        ImGui::Text("%s", p->isActive()?"true":"false");
        ImGui::PopID();
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();
}

#endif

void USCGUI::errorPopup(const char* errorTxt, ...) {
  va_list args;
  va_start(args, errorTxt);
  size_t len = vsnprintf(NULL, 0, errorTxt, args);
  va_end(args);
  errorText.reserve(len+1);
  errorText.resize(len);
  vsnprintf(&errorText[0], len, errorTxt, args);
  ImGui::OpenPopup("Error##ERRPOPUP");
}

void USCGUI::readConfig() {
  // settings
  settingsParams.readConfig(config);

  YAML::Node guiNode = config->getConfig("gui", YAML::Node());
  YAML::Node woNode = guiNode["windows"];

  // windows
  wo.settingsOpen = getConfig(woNode, "settingsOpen", false);
  wo.dataIOConfigOpen = getConfig(woNode, "dataIOConfigOpen", false);

  // internal vars

  windowWidth  = getConfig(guiNode, "windowWidth", 1280);
  windowHeight = getConfig(guiNode, "windowHeight", 720);

  fullscreen = getConfig(guiNode, "fullscreen", false);

  if (config->exists("layoutFile"))
    layoutFile = getConfig<string>(guiNode, "layoutFile", "");

  // scopes
  char key[256];
  snprintf(key, 256, "scope0");
  for (int i=1; config->exists(key); i++) {
    try {
      scopeWindows.push_back(ScopeWindow(data, i-1));
      scopeWindows[i-1].loadFromNode(config->getConfig<YAML::Node>(key, YAML::Node()));
      printf(INFO_MSG "GUI: loaded scope %d" MSG_END, i-1);
    } catch(std::exception& e) {
      printf(ERROR_MSG "GUI: failed to read config %s: %s" MSG_END, key, e.what());
    }
    snprintf(key, 256, "scope%d", i);
  }
}

void USCGUI::writeConfig() {
  // settings
  settingsParams.writeConfig(config);

  // windows
  YAML::Node woNode;
  woNode["settingsOpen"] = wo.settingsOpen;
  woNode["dataIOConfigOpen"] = wo.dataIOConfigOpen;

  // internal vars
  YAML::Node guiNode;
  SDL_GetWindowSize(rd->getWindow(), &windowWidth, &windowHeight);
  guiNode["windowWidth"] = windowWidth;
  guiNode["windowHeight"] = windowHeight;

  guiNode["fullscreen"] = fullscreen;

  if (!layoutFile.empty())
    ImGui::SaveIniSettingsToDisk(layoutFile.c_str());
  guiNode["layoutFile"] = layoutFile;

  guiNode["windows"] = woNode;
  config->setConfig<YAML::Node>("gui", guiNode);

  // scopes
  char strbuf[256];
  for (int i=0; i<scopeWindows.size(); i++) {
    snprintf(strbuf, 256, "scope%d", i);
    config->setConfig<YAML::Node>(strbuf, scopeWindows[i].saveToNode());
  }
}

USCGUI::~USCGUI() {
  if (rd) rd->destroyRender();
  DELETE_PTR(rd)
  for (int i=0; i<scopeWindows.size(); i++) {
    for (int j=0; j<scopeWindows[i].channels.size(); j++) {
      delete scopeWindows[i].channels.at(j);
    }
    scopeWindows[i].channels.clear();
  }
}
