GH-3963
Age-dependent HINT transitions should not assume start day = 0!
12/6/2019

The problem was that the campaign events added for the Age_Bin IP assumed a Start_Day=0.
This was resolved by getting the Start_Time from the config.

The test first started with a Start_Time=0 and events in the campaign starting at Start_Day=0.
We then changed the Start_Time=10 and incremented the Start_Day's by 10.  Before the changes
InsetChart was quite different.  After the changes, the InsetChart was the same.
