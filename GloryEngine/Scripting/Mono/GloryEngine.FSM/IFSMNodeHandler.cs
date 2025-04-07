namespace GloryEngine.FSM
{
    /// <summary>
    /// Handler interface for state changes
    /// </summary>
    public interface IFSMNodeHandler
    {
        #region Methods

        /// <summary>
        /// Called when the state machine transitions to this state
        /// </summary>
        /// <param name="instance">State machine instance</param>
        void OnStateEntry(FSMInstance instance);
        /// <summary>
        /// Called when the state machine transitions from this state
        /// </summary>
        /// <param name="instance">State machine instance</param>
        void OnStateExit(FSMInstance instance);

        #endregion
    }
}
