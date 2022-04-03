#!/usr/bin/python

lessons = []
text = []
lesson1 = {}
lesson1["title"] = "Welcome to the DTK Tutorial. <br>"
lesson1["section"] = "Input Demographics.<br>"
lesson1["page"] = "1 <br>"
lesson1["topic"] = "Built-In Demographics <br>"
lesson1["params"] = [ "Enable_Demographics_Builtin", "Default_Torus_Size", "Default_Torus_Population" ]
lesson1["blurb"] = """
Set Enable_Demographics_Builtin to 1 (true) if you want to use the built-in demographics configuration. This is useful for a 'quickstart' experience if you don't have a valid DTK demographics input file yet but want to see the program run. Set Enable_Demographics_Builtin to 0 (false) when you want to switch to your own demographics input file, as specified with the Demographics_Filenames parameter. If Enable_Demographics_Builtin is on, you'll also need to specify Default_Torus_Size and Default_Torus_Population. NOTE: THIS ISN'T PART OF THE DEMO YET.
"""
lessons.append( lesson1 )

lesson2 = {}
lesson2["title"] = "Welcome to the DTK Tutorial. <br>"
lesson2["section"] = "Input Demographics.<br>"
lesson2["page"] = "2 <br>"
lesson2["topic"] = "Population Size<br>"
lesson2["params"] = [ "InitialPopulation", "Population_Scale_Type", "x_Base_Population" ]
lesson2["blurb"] = """
The first thing you might want to set is the initial (human) population. This is primarily done via the InitialPopulation parameter in the demographics file. That value you be multiplied (up or down) using the x_Base_Population in the configs. To activate x_Base_Population, you need to set Population_Scale_Type to 'FIXED_SCALING'. Note that this demo is just for a single node. The InitialPopulation is a per-node value, and the multiplier is simulation-wide (all node initial populations will be multiplied by this value).
"""
lessons.append( lesson2 )

lesson3 = {}
lesson3["title"] = "Welcome to the DTK Tutorial. <br>"
lesson3["section"] = "Input Demographics.<br>"
lesson3["page"] = "3 <br>"
lesson3["topic"] = "Individual Subsampling (None)<br>"
lesson3["params"] = [ "Individual_Sampling_Type" ]
lesson3["blurb"] = """
Blurb
"""
lessons.append( lesson3 )

lesson4 = {}
lesson4["title"] = "Welcome to the DTK Tutorial. <br>"
lesson4["section"] = "Input Demographics.<br>"
lesson4["page"] = "4 <br>"
lesson4["topic"] = "Individual Subsampling (Fixed)<br>"
lesson4["params"] = [ "Individual_Sampling_Type", "Base_Individual_Sample_Rate" ]
lesson4["blurb"] = """
Blurb
"""
lessons.append( lesson4 )

lesson5 = {}
lesson5["title"] = "Welcome to the DTK Tutorial. <br>"
lesson5["section"] = "Input Demographics.<br>"
lesson5["page"] = "5 <br>"
lesson5["topic"] = "Individual Subsampling (Adaptive by Immunity)<br>"
lesson5["params"] = [ "Individual_Sampling_Type", "Base_Individual_Sample_Rate", "Immune_Threshold_For_Downsampling", "Relative_Sample_Rate_Immune" ]
lesson5["blurb"] = """
Blurb
"""
lessons.append( lesson5 )
