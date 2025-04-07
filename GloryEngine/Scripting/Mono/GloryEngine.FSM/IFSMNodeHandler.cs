namespace GloryEngine.FSM
{
    public interface IFSMNodeHandler
    {
        #region Methods

        void OnStateEntry(FSMInstance instance);
        void OnStateExit(FSMInstance instance);

        #endregion
    }
}
