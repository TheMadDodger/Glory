#include "ProfilerTimeline.h"

namespace Glory
{
    // interface with sequencer

    ProfilerTimeline::ProfilerTimeline() : mFrameMin(0), mFrameMax(0) {}

    ProfilerTimeline::~ProfilerTimeline() {}

    int ProfilerTimeline::GetFrameMin() const
    {
        return mFrameMin;
    }

    int ProfilerTimeline::GetFrameMax() const
    {
        return mFrameMax;
    }

    int ProfilerTimeline::GetItemCount() const { return (int)myItems.size(); }

    int ProfilerTimeline::GetItemTypeCount() const { return 1; }
    const char* ProfilerTimeline::GetItemTypeName(int typeIndex) const { return "test"; }
    const char* ProfilerTimeline::GetItemLabel(int index) const
    {
        static char tmps[512];
        snprintf(tmps, 512, "[%02d] %s", index, "test");
        return tmps;
    }

    void ProfilerTimeline::Get(int index, int** start, int** end, int* type, unsigned int* color)
    {
        MySequenceItem& item = myItems[index];
        if (color)
            *color = 0xFFAA8080; // same color for everyone, return color based on type
        if (start)
            *start = &item.mFrameStart;
        if (end)
            *end = &item.mFrameEnd;
        if (type)
            *type = item.mType;
    }
    void ProfilerTimeline::Add(int type) { myItems.push_back(MySequenceItem{ type, 0, 10, false }); };
    void ProfilerTimeline::Del(int index) { myItems.erase(myItems.begin() + index); }
    void ProfilerTimeline::Duplicate(int index) { myItems.push_back(myItems[index]); }

    size_t ProfilerTimeline::GetCustomHeight(int index) { return myItems[index].mExpanded ? 300 : 0; }

    void ProfilerTimeline::DoubleClick(int index)
    {
        if (myItems[index].mExpanded)
        {
            myItems[index].mExpanded = false;
            return;
        }
        for (auto& item : myItems)
            item.mExpanded = false;
        myItems[index].mExpanded = !myItems[index].mExpanded;
    }

    void ProfilerTimeline::CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect)
    {
        static const char* labels[] = { "Translation", "Rotation" , "Scale" };

        //rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
        //rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
        //draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
        //for (int i = 0; i < 3; i++)
        //{
        //    ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
        //    ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
        //    draw_list->AddText(pta, rampEdit.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
        //    if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
        //        rampEdit.mbVisible[i] = !rampEdit.mbVisible[i];
        //}
        //draw_list->PopClipRect();
        //
        //ImGui::SetCursorScreenPos(rc.Min);
        //ImCurveEdit::Edit(rampEdit, rc.Max - rc.Min, 137 + index, &clippingRect);
    }

    void ProfilerTimeline::CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect)
    {
        //rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
        //rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
        //draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
        //for (int i = 0; i < 3; i++)
        //{
        //    for (int j = 0; j < rampEdit.mPointCount[i]; j++)
        //    {
        //        float p = rampEdit.mPts[i][j].x;
        //        if (p < myItems[index].mFrameStart || p > myItems[index].mFrameEnd)
        //            continue;
        //        float r = (p - mFrameMin) / float(mFrameMax - mFrameMin);
        //        float x = ImLerp(rc.Min.x, rc.Max.x, r);
        //        draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
        //    }
        //}
        //draw_list->PopClipRect();
    }
}