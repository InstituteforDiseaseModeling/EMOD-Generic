import json
import os
import unittest
import dtk_Individual_Property_Support as d_ipp

class Individual_Property_Support_Unittest(unittest.TestCase):
    class TestDefaults:
        config_name = "IndividualProperty_TestConfig"
        demographics_filenames = ["demographics_fake.json", "demographics_fake_overlay.json"]
        enable_property_output = 1
        enable_hint = 1
        campaign_filename = "campaign_fake.json"
        start_time = 1
        timestep_size = 1
        enable_interventions = 1

    def setUp(self):
        self._testFiles = []
        self.testConfigParams = None
        self.testOverlay = None
        pass

    def tearDown(self):
        for f in self._testFiles:
            if os.path.isfile(f):
                os.unlink(f)
        pass

    def _write_fake_content(self, json_object, filename):
        with open(filename, 'w') as outfile:
            json.dump(json_object, outfile, indent=4)
        self._testFiles.append(filename)

    def write_fake_config_file(self, config_filename,
                               config_name=TestDefaults.config_name,
                               demographics_filenames=TestDefaults.demographics_filenames,
                               enable_property_output=TestDefaults.enable_property_output,
                               enable_hint=TestDefaults.enable_hint,
                               campaign_filename=TestDefaults.campaign_filename,
                               start_time=TestDefaults.start_time,
                               timestep_size=TestDefaults.timestep_size,
                               enable_interventions=TestDefaults.enable_interventions):
        config_file_object = {}
        params = {}
        params[d_ipp.ConfigKeys.CONFIG_NAME] = config_name
        params[d_ipp.ConfigKeys.ENABLE_INTERVENTIONS] = enable_interventions
        params[d_ipp.ConfigKeys.TIMESTEP_SIZE] = timestep_size
        params[d_ipp.ConfigKeys.START_TIME] = start_time
        params[d_ipp.ConfigKeys.ENABLE_PROPERTY_OUTPUT] = enable_property_output
        params[d_ipp.ConfigKeys.ENABLE_HINT] = enable_hint
        params[d_ipp.ConfigKeys.CAMPAIGN_FILENAME] = campaign_filename
        params[d_ipp.ConfigKeys.DEMOGRAPHICS_FILENAMES] = demographics_filenames
        self.testConfigParams = params
        config_file_object[d_ipp.ConfigKeys.PARAMETERS_KEY] = params

        self._write_fake_content(config_file_object, config_filename)

    def fake_demographics_overlay(self):
        metadata = {}
        metadata["DateCreated"] = "Sun Sep 25 23:19:55 2011"

        nodes = []
        my_node = {}
        my_node["NodeID"] = 1
        nodes.append(my_node)

        defaults = {}
        defaults["IndividualProperties"] = []

        self.testOverlay = {"Metadata":metadata,
                            "Nodes":nodes,
                            "Defaults":defaults}
        pass

    def add_fake_IP_to_overlay(self,property):
        if not self.testOverlay:
            self.fake_demographics_overlay()
        self.testOverlay["Defaults"]["IndividualProperties"].append(property)

    def write_fake_demographics_overlay(self, filename):
        if not self.testOverlay:
            self.fake_demographics_overlay()
        self._write_fake_content(self.testOverlay, filename=filename)

    def make_fake_access_property(self):
        property_access = {}
        property_access[d_ipp.DemographicsKeys.PropertyKeys.PROPERTY_NAME] = "Accessibility"
        property_access[d_ipp.DemographicsKeys.PropertyKeys.PROPERTY_VALUES] = ["Easy", "Hard"]
        property_access[d_ipp.DemographicsKeys.PropertyKeys.INITIAL_DISTRIBUTION] = [0.7, 0.3]
        property_access[d_ipp.DemographicsKeys.PropertyKeys.TRANSITIONS_KEY] = []
        return property_access

    def make_fake_risk_property(self):
        property_risk = {}
        property_risk[d_ipp.DemographicsKeys.PropertyKeys.PROPERTY_NAME] = "Risk"
        property_risk[d_ipp.DemographicsKeys.PropertyKeys.PROPERTY_VALUES] = ["Low", "High"]
        property_risk[d_ipp.DemographicsKeys.PropertyKeys.INITIAL_DISTRIBUTION] = [0.8, 0.2]
        property_risk[d_ipp.DemographicsKeys.PropertyKeys.TRANSITIONS_KEY] = []
        return property_risk

    def make_fake_place_property(self):
        property_place = {}
        property_place[d_ipp.DemographicsKeys.PropertyKeys.PROPERTY_NAME] = "Place"
        property_place[d_ipp.DemographicsKeys.PropertyKeys.PROPERTY_VALUES] = ["Home", "Work", "School", "Mall", "Library"]
        property_place[d_ipp.DemographicsKeys.PropertyKeys.INITIAL_DISTRIBUTION] = [0.50, 0.25, 0.14, 0.07, 0.04]
        property_place[d_ipp.DemographicsKeys.PropertyKeys.TRANSITIONS_KEY] = []
        return property_place

    def build_three_property_overlay(self, filename):
        property_access = self.make_fake_access_property()
        property_risk = self.make_fake_risk_property()
        property_place = self.make_fake_place_property()

        self.add_fake_IP_to_overlay(property_access)
        self.add_fake_IP_to_overlay(property_risk)
        self.add_fake_IP_to_overlay(property_place)
        self.write_fake_demographics_overlay(filename)

        return property_access, property_place, property_risk

    def build_access_risk_overlay(self, filename):
        property_risk = self.make_fake_risk_property()
        property_access = self.make_fake_access_property()

        self.add_fake_IP_to_overlay(property_risk)
        self.add_fake_IP_to_overlay(property_access)
        self.write_fake_demographics_overlay(filename)
        return property_access, property_risk

    def build_access_overlay(self, filename):
        property_access = self.make_fake_access_property()

        self.add_fake_IP_to_overlay()
        self.write_fake_demographics_overlay(filename)

    def test_load_config_file(self):
        fake_config_filename = "fake_config.json"
        self.write_fake_config_file(config_filename=fake_config_filename)
        config_object = d_ipp.load_config_file(filename=fake_config_filename)
        for key in self.testConfigParams:
            self.assertEqual(self.testConfigParams[key], config_object[key])

    def test_get_properties_from_demographics(self, debug=False):
        overlay_filename = "test_demographics_overlay.json"
        property_access, property_place, property_risk = self.build_three_property_overlay(overlay_filename)

        returned_properties = d_ipp.get_properties_from_demographics(overlay_filename)
        self.assertEqual(3, len(returned_properties), "there should be three properties")

        name_key = d_ipp.DemographicsKeys.PropertyKeys.PROPERTY_NAME
        for p in returned_properties:
            checkMe = None
            if p[name_key] == "Risk":
                checkMe = property_risk
            elif p[name_key] == "Accessibility":
                checkMe = property_access
                pass
            elif p[name_key] == "Place":
                checkMe = property_place
            else:
                self.assertTrue(False, f"property should have been Risk, Accessibility or Place, got {p}")
            # Now we know the property to check.
            if debug:
                print(f"CHECKME IS {checkMe}\n")
            for k in checkMe:
                if debug:
                    print (f"KEY IN QUESTION IS: {k}\n")
                self.assertEqual(p[k], checkMe[k], msg=f"Returned property {p} and json property {checkMe} should"
                                 f" have same value for {k}")

    def test_get_property_strings(self):
        access_property = self.make_fake_access_property()
        values_to_find = access_property[d_ipp.DemographicsKeys.PropertyKeys.PROPERTY_VALUES]
        property_name = access_property[d_ipp.DemographicsKeys.PropertyKeys.PROPERTY_NAME]

        property_strings = d_ipp.build_strings_for_property(access_property)
        self.assertEqual(len(values_to_find),
                         len(property_strings))
        for ps in property_strings:
            self.assertIn(member=property_name, container=ps)
            split_ps = ps.split(':')
            self.assertEqual(len(split_ps), 2)
            self.assertIn(member=split_ps[1], container=values_to_find)
            values_to_find.remove(split_ps[1])
        self.assertEqual(len(values_to_find), 0)


if __name__ == "__main__":
    unittest.main()
