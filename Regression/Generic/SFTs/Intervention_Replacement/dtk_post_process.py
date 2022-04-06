#!/usr/bin/python
import os
if __name__ == '__main__':
    from pathlib import Path
    import sys
    os.chdir(str(Path(sys.argv[0]).parent))
    sys.path.append( str(Path('../../../shared_embedded_py_scripts/').resolve().absolute()) )

from dtk_test.dtk_sft_class import SFT, arg_parser


"""
A test for Intervention Replacement Feature in DTK
Spec is here https://github.com/InstituteforDiseaseModeling/EMOD-docs/blob/master/specs/Interventions-spec.rst#configuration-parameters
Script to generated the follow campaign.json is here: https://github.com/InstituteforDiseaseModeling/emodpy-generic/blob/master/tests/feature_testing/test_intervention_replacement.py
DAD  = Dont_Allow_Duplicates
EIR  = Enable_Intervention_Replacement

Campaign built using the pattern:

Time       Event

10*N+1     Vaccine1            -  30% Acquision Blocking          (Expires after 6 days)
10*N+2     Vaccine2            -  60% Acquision Blocking          (Expires after 6 days)
10*N+3     OutbreakIndividual  - 100% Coverage                    (Infectious duration = 5 days; Infectivity = 0)
10*N+5     Examine Prevalence  - Ought to be ~70%, ~40%, or ~28%




N        Vaccine1            Vaccine2         DAD      EIR      Expected Prevalence

 0       Name = Default      Name = Default    0        0              28%
 1       Name = Default      Name = Default    0        1              28% 
 2       Name = Default      Name = Default    1        0              70%
 3       Name = Default      Name = Default    1        1              40%

 4       Name = Alice        Name = Default    0        0              28%
 5       Name = Alice        Name = Default    0        1              28% 
 6       Name = Alice        Name = Default    1        0              28%
 7       Name = Alice        Name = Default    1        1              28%

 8       Name = Alice        Name = Alice      0        0              28%
 9       Name = Alice        Name = Alice      0        1              28% 
10       Name = Alice        Name = Alice      1        0              70%
11       Name = Alice        Name = Alice      1        1              40%

12       Name = Alice        Name = Bob        0        0              28%
13       Name = Alice        Name = Bob        0        1              28% 
14       Name = Alice        Name = Bob        1        0              28%
15       Name = Alice        Name = Bob        1        1              28%

"""


class InterventionReplacementTest(SFT):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.json_report_name = "InsetChart.json"

    # overwrite the test method
    def test(self):
        self.parse_json_report(channel_names=[Channels.Infected])
        inset_chart = self.json_report

        efficacy_1 = 0.3
        efficacy_2 = 0.6
        initial_immunity = 1.0

        multiplicative_efficacy = efficacy_1 + (initial_immunity - efficacy_1) * efficacy_2
        multiplicative_prevalence = 1.0 - multiplicative_efficacy

        set_sample = inset_chart.df[Channels.Infected][5::10].tolist()
        set_expect = [multiplicative_prevalence, multiplicative_prevalence, 1.0 - efficacy_1, 1.0 - efficacy_2,
                      multiplicative_prevalence, multiplicative_prevalence, multiplicative_prevalence, multiplicative_prevalence,
                      multiplicative_prevalence, multiplicative_prevalence, 1.0 - efficacy_1, 1.0 - efficacy_2,
                      multiplicative_prevalence, multiplicative_prevalence, multiplicative_prevalence, multiplicative_prevalence]
        for k1 in range(len(set_expect)):
            self.msg.append(f'Comparing actual prevalence with expected prevalence for time {10*k1 + 5}:\n')
            if round(float(set_sample[k1]), 2) != set_expect[k1]:
                self.success = False
                self.msg.append('BAD: ')
            else:
                self.msg.append('GOOD: ')
            self.msg.append(f"Actual, Expected:  {set_sample[k1]:4.2f}, {set_expect[k1]:4.2f}.\n")


class Channels:
    Infected = "Infected"


def application(output_folder="output", my_arg=None):
    if not my_arg:
        my_sft = InterventionReplacementTest()
    else:
        my_sft = InterventionReplacementTest(
            output=my_arg.output, stdout=my_arg.stdout, json_report=my_arg.json_report, event_csv=my_arg.event_csv,
            config=my_arg.config, campaign=my_arg.campaign, report_name=my_arg.report_name, debug=my_arg.debug)
    my_sft.run()


if __name__ == "__main__":
    # execute only if run as a script
    my_arg = arg_parser()
    application(my_arg=my_arg)
