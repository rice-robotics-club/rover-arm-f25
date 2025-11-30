# Logging all the times AI has hallucinated so it doesn't just keep spewing bs solutions 
1. Polling or any other function that requires the executor to spin **WILL** cause the code to crash\
This is because there already is an executor

# Also logging any interesting things I found during my AI assisted escapades
1. CallbackGroups -> Debating throwing both the UpdateGoalItem service and goalPose topic into a MutuallyExclusive\
CallbackGroup since there's no point reading the goalPose until the new Item has been received by VIsion