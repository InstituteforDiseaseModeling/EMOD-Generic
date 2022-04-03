
import dtk_common_setup

dtk_common_setup.doitall(
	'dtk_nodedemog',
        src_list = [
            'nodedemog_module.cpp',
            '../../Eradication/BroadcasterImpl.cpp',
            '../../Eradication/Node.cpp',
            '../../Eradication/NodeDemographics.cpp',
            '../../Eradication/NodeEventContext.cpp',
            '../../Eradication/Individual.cpp',
            '../../Eradication/Susceptibility.cpp',
            '../../Eradication/Infection.cpp',
            '../../Eradication/InterventionsContainer.cpp',
            '../../Eradication/StrainIdentity.cpp',
            '../../Eradication/Instrumentation.cpp',
            '../../Eradication/TransmissionGroupsBase.cpp',
            '../../Eradication/StrainAwareTransmissionGroups.cpp', 
            '../../Eradication/TransmissionGroupsFactory.cpp'
        ]
)

