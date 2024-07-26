#include "gui.h"

void USCGUI::drawNodeSpace() {
  ImGui::Begin("process space", &wo.nodeSpaceOpen);
    static ImVector<ImVec2> points;
    static bool opt_enable_grid = true;
    static bool adding_line = false;

    ImGui::Checkbox("Enable grid", &opt_enable_grid);

    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
    if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
    if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

    // Draw border and background color
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool is_hovered = ImGui::IsItemHovered(); // Hovered
    const bool is_active = ImGui::IsItemActive();   // Held
    const ImVec2 origin(canvas_p0.x + ns.camera.x, canvas_p0.y + ns.camera.y); // Lock scrolled origin
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

    // Add first and second point
    if (is_hovered && !adding_line && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      points.push_back(mouse_pos_in_canvas);
      points.push_back(mouse_pos_in_canvas);
      adding_line = true;
    }
    if (adding_line) {
      points.back() = mouse_pos_in_canvas;
      if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) adding_line = false;
    }

    // Pan (we use a zero mouse threshold when there's no context menu)
    // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
    const float mouse_threshold_for_pan = -1.0f;
    if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan)) {
      ns.camera.x += io.MouseDelta.x;
      ns.camera.y += io.MouseDelta.y;
    }

    // Context menu (under default mouse threshold)
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (drag_delta.x == 0.0f && drag_delta.y == 0.0f)
      ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    if (ImGui::BeginPopup("context")){
      if (adding_line) points.resize(points.size() - 2);
      adding_line = false;
      // if (ImGui::MenuItem("Remove one", NULL, false, points.Size > 0)) { points.resize(points.size() - 2); }
      // if (ImGui::MenuItem("Remove all", NULL, false, points.Size > 0)) { points.clear(); }
      if (ImGui::BeginMenu("Add node", ns.nodeCount < 256)) {
        for (unsigned short i = 0; i < PNODE_COUNT; i++) {
          // if (ImGui::MenuItem("",NULL)) 
        }
        ImGui::EndMenu();
      }
      ImGui::EndPopup();
    }

    // Draw grid + all lines in the canvas
    draw_list->PushClipRect(canvas_p0, canvas_p1, true);
    if (opt_enable_grid) {
      const float GRID_STEP = 64.0f;
      for (float x = fmodf(ns.camera.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
        draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
      for (float y = fmodf(ns.camera.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
        draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
    }
    for (unsigned short i = 0; i < ns.nodeCount; i++) {
      drawNode(ns.nodes[i]);
    }
    draw_list->PopClipRect();

  ImGui::End();
}

void USCGUI::drawNode(node* n) {
  if (ImGui::BeginChild(n->node->getName())) {
    for (unsigned char i = 0; i < 4; i++) {
      ImGui::SliderFloat(
        n->node->params[i].name,
        &n->node->params[i].value,
        n->node->params[i].vMin,
        n->node->params[i].vMax,
        "%g");
    }
    ImGui::EndChild();
  }
}

void USCGUI::addNode(ProcessNodes p) {
  ns.nodeCount++;
  ns.nodes[ns.nodeCount] = new node;
  ns.nodes[ns.nodeCount]->node = ai->getAudioProcess()->addNode(p);
}

void USCGUI::removeNode(unsigned short n) {
  delete ns.nodes[n]->node;
  ns.nodes[n]->node = NULL;
  ns.nodes.erase(ns.nodes.begin()+n);
  ns.nodeCount--;
}
