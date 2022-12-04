#pragma once
#include "Undo.h"

namespace Glory::Editor
{
    class BasePropertyAction : public IAction
    {
    public:
        GLORY_EDITOR_API BasePropertyAction(const std::string& propertyName);
        GLORY_EDITOR_API virtual ~BasePropertyAction();

    protected:
        virtual void SetOldValue(void* pMemberPoiner) = 0;
        virtual void SetNewValue(void* pMemberPoiner) = 0;

    private:
        virtual GLORY_EDITOR_API void OnUndo(const ActionRecord& actionRecord);
        virtual GLORY_EDITOR_API void OnRedo(const ActionRecord& actionRecord);

    private:
        const std::string& m_PropertyName;
    };

    template<typename T>
    class PropertyAction : public BasePropertyAction
    {
    public:
        GLORY_EDITOR_API PropertyAction(const std::string& propertyName, T& oldValue, T& newValue) : m_OldValue(oldValue), m_NewValue(newValue), BasePropertyAction(propertyName) {}
        GLORY_EDITOR_API virtual ~PropertyAction() {}

    private:
        virtual void SetOldValue(void* pMemberPoiner)
        {
            T* pValuePointer = (T*)pMemberPoiner;
            *pValuePointer = m_OldValue;
        }

        virtual void SetNewValue(void* pMemberPoiner)
        {
            T* pValuePointer = (T*)pMemberPoiner;
            *pValuePointer = m_NewValue;
        }

    private:
        T m_OldValue;
        T m_NewValue;
    };
}
