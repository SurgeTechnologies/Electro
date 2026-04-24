//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Events/Event.hpp"

namespace Electro
{
    // Base class for all touch pointer events.
    // Each touch point is identified by an integer finger ID (0-based) assigned
    // by the OS for the lifetime of a single gesture.  Coordinates are in
    // screen pixels from the top-left corner of the window.
    class TouchEvent : public Event
    {
    public:
        int  GetFingerID() const { return mFingerID; }
        float GetX()       const { return mX; }
        float GetY()       const { return mY; }

        EVENT_CLASS_CATEGORY(EventCategoryTouch | EventCategoryInput)
    protected:
        TouchEvent(int fingerID, float x, float y)
            : mFingerID(fingerID), mX(x), mY(y) {}

        int   mFingerID;
        float mX, mY;
    };

    // A new finger has touched the screen.
    class TouchBeganEvent : public TouchEvent
    {
    public:
        TouchBeganEvent(int fingerID, float x, float y)
            : TouchEvent(fingerID, x, y) {}

        String ToString() const override
        {
            std::stringstream ss;
            ss << "TouchBeganEvent: finger=" << mFingerID
               << " (" << mX << ", " << mY << ")";
            return ss.str();
        }

        EVENT_CLASS_TYPE(TouchBegan)
    };

    // An existing finger has moved across the screen.
    class TouchMovedEvent : public TouchEvent
    {
    public:
        TouchMovedEvent(int fingerID, float x, float y, float dx, float dy)
            : TouchEvent(fingerID, x, y), mDeltaX(dx), mDeltaY(dy) {}

        float GetDeltaX() const { return mDeltaX; }
        float GetDeltaY() const { return mDeltaY; }

        String ToString() const override
        {
            std::stringstream ss;
            ss << "TouchMovedEvent: finger=" << mFingerID
               << " (" << mX << ", " << mY << ")"
               << " delta=(" << mDeltaX << ", " << mDeltaY << ")";
            return ss.str();
        }

        EVENT_CLASS_TYPE(TouchMoved)
    private:
        float mDeltaX, mDeltaY;
    };

    // A finger has been lifted from the screen.
    class TouchEndedEvent : public TouchEvent
    {
    public:
        TouchEndedEvent(int fingerID, float x, float y)
            : TouchEvent(fingerID, x, y) {}

        String ToString() const override
        {
            std::stringstream ss;
            ss << "TouchEndedEvent: finger=" << mFingerID
               << " (" << mX << ", " << mY << ")";
            return ss.str();
        }

        EVENT_CLASS_TYPE(TouchEnded)
    };
}
