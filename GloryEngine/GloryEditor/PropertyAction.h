#pragma once
#include "Undo.h"

namespace Glory::Editor
{
    class BasePropertyAction : public IAction
    {
    public:
        BasePropertyAction(const std::string& propertyName);
        ~BasePropertyAction();

    protected:
        virtual void SetOldValue(void* pMemberPoiner) = 0;
        virtual void SetNewValue(void* pMemberPoiner) = 0;

    private:
        virtual void OnUndo(const ActionRecord& actionRecord);
        virtual void OnRedo(const ActionRecord& actionRecord);

    private:
        const std::string& m_PropertyName;
    };

    template<typename T>
    class PropertyAction : public BasePropertyAction
    {
    public:
        PropertyAction(const std::string& propertyName, T& oldValue, T& newValue) : m_OldValue(oldValue), m_NewValue(newValue), BasePropertyAction(propertyName) {}
        virtual ~PropertyAction() {}

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
