from dtk_ep4 import dtk_pre_process_w5ml

def application( config_name ):
    dtk_pre_process_w5ml.application( "campaign.w5ml" )


if __name__ == "__main__":
    application( "config.json" )
