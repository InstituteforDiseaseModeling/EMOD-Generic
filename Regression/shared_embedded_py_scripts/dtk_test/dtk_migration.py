#! /usr/bin/env python3

import json
import struct
import sys


KEY_GENDERDATATYPE = 'GenderDataType'
KEY_AGESYEARS = 'AgesYears'
NUM_SEXES = 2

MALE = 0
FEMALE = 1


class MigrationFile(object):
    '''
    Represents a DTK style migration file. Should handle single layer (same rates for all demographics)
    as well as multi-layer (different rates by sex and/or age).
    Does _not_ handle LINEAR_INTERPOLATION, only PIECEWISE_CONSTANT.
    '''

    def __init__(self, filename):
        '''
        Construct with path to binary migration file name. Associated metadata file (filename + '.json')
        should be in the same directory.
        '''

        self.__load_metadata( filename + '.json' )
        self.__load_data( filename )

        return

    def __load_metadata( self, filename ):

        with open(filename, 'r') as handle:
            jason = json.load(handle)

        metadata = jason['Metadata']

        self.node_count = metadata['NodeCount']
        self.data_value_count = metadata['DatavalueCount']
        self.gender_count = 1 if (KEY_GENDERDATATYPE not in metadata) or (metadata[KEY_GENDERDATATYPE] == 'SAME_FOR_BOTH_GENDERS') else 2
        self.age_bins = [125] if KEY_AGESYEARS not in metadata else metadata[KEY_AGESYEARS]

        self.__parse_node_offsets( jason['NodeOffsets'] )

        return

    def __parse_node_offsets( self, offsets_string ):

        self.node_offsets = {}
        for i in range(0, len(offsets_string), 16):
            node_id = int(offsets_string[i:i+8], base=16)
            node_offset = int(offsets_string[i+8:i+16], base=16)
            self.node_offsets[node_id] = node_offset

        return

    def __load_data( self, filename ):

        destination_format = '<' + str(self.data_value_count) + 'I' # uint32_t
        rate_format = '<' + str(self.data_value_count) + 'd'        # double

        self.rates_by_sex = {}

        swap = {offset:id for id, offset in self.node_offsets.items()}
        offsets = sorted(list(swap.keys()))
        node_ids = {swap[offset] for offset in offsets}

        with open(filename, 'rb') as handle:

            for sex in range(self.gender_count):
                rates_by_age = {}
                for bin in range(len(self.age_bins)):
                    rates_by_node = {}
                    for id in node_ids:
                        bytes = handle.read( 4 * self.data_value_count )
                        destinations = struct.unpack( destination_format, bytes )
                        bytes = handle.read( 8 * self.data_value_count )
                        rates = struct.unpack( rate_format, bytes )
                        rates_by_node[id] = { d:r for d, r in zip( destinations, rates ) if r > 0 }
                    rates_by_age[bin] = rates_by_node
                self.rates_by_sex[sex] = rates_by_age

        return

    def rates(self, age=0, sex=0):
        '''
        Returns rate(s) of migration for a given demographic (age and sex) as a dictionary
        keyed on source node id. Values are a second dictionary keyed on destination node id.
        Values are daily rate(s) of migration.
        '''
        age_index = self.__index_for_age(age)
        sex_index = self.__index_for_sex(sex)

        return self.rates_by_sex[sex_index][age_index]

    def __index_for_age(self, age):
        index = 0
        while age > self.age_bins[index]:
            index += 1

        if index >= len(self.age_bins):
            raise RuntimeError(f'Age {age} is out of range for age bins {self.age_bins}')

        return index

    def __index_for_sex(self, sex):
        if sex < 0 or sex >= NUM_SEXES:
            raise RuntimeError(f'Sex {sex} is out of range [0, 1]')

        index = sex % self.gender_count

        return index

    def to_json(self):
        '''
        Output all migration information as a JSON formatted string.
        TODO - match input format used by convert_json_to_bin.py (for roundtripping).
        See DtkInputs/MigrationTest.
        '''
        jason = '{"Implemented":false}'
        return jason


if __name__ == '__main__':

    # migration = MigrationFile( 'e:/src/ifdm/inputs/garki/Garki_30arcsec_local_migration.bin')
    # rates = migration.rates()
    # node_id = list(rates.keys())[0]
    # print('A node from the migration file: {0}'.format(node_id))
    # print('Nodes reachable from node {0}: {1}'.format(node_id, list(rates[node_id].keys())))

    # both_genders = MigrationFile( '../componenttests/testdata/migrationtest/testbothgenders.bin')
    # middle_age = both_genders.rates(age=50)
    # node_id = list(middle_age.keys())[0]
    # total = sum(middle_age[node_id].values())
    # print('Migration out of node {0}: {1}'.format(node_id, middle_age[node_id]))
    # print('Total rate of migration from node {0} = {1}'.format(node_id, total))

    # each_gender = MigrationFile( '../componenttests/testdata/migrationtest/testeachgender.bin')
    # young_men = each_gender.rates(age=25, sex=MALE)
    # node_id = list(young_men.keys())[0]
    # print('Options for young men:   {0}'.format(young_men[node_id]))
    # young_women = each_gender.rates(age=25, sex=FEMALE)
    # node_id = list(young_women.keys())[0]
    # print('Options for young women: {0}'.format(young_women[node_id]))

    sys.exit(0)
