#pragma once
#include <ImSequencer.h>
#include <imgui.h>
#include <vector>

namespace Glory
{
	class ProfilerTimeline : public ImSequencer::SequenceInterface
	{
    public:
        ProfilerTimeline();
        virtual ~ProfilerTimeline();

    public:
        virtual int GetFrameMin() const;
        virtual int GetFrameMax() const;
        virtual int GetItemCount() const;
        virtual int GetItemTypeCount() const;
        virtual const char* GetItemTypeName(int typeIndex) const;
        virtual const char* GetItemLabel(int index) const;
        virtual void Get(int index, int** start, int** end, int* type, unsigned int* color);
        virtual void Add(int type);
        virtual void Del(int index);
        virtual void Duplicate(int index);
        virtual size_t GetCustomHeight(int index);
        virtual void DoubleClick(int index);
        virtual void CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect);
        virtual void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect);

    public:
        int mFrameMin, mFrameMax;
        struct MySequenceItem
        {
            int mType;
            int mFrameStart, mFrameEnd;
            bool mExpanded;
        };

        std::vector<MySequenceItem> myItems;
	};
}
