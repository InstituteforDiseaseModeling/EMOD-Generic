import json, sys, os, argparse


def RenameChannelFromInsetChart( filename, oldName, newName ):
    
    with open( filename, "r" ) as file:
        json_data = json.load( file )
    
    if ("Channels" not in json_data.keys()) or ("Header" not in json_data.keys()):
        print(filename + "not InsetChart")
        return
    
    if oldName not in json_data["Channels"].keys():
        return
        
    if newName in json_data["Channels"].keys():
        return
        
    print("Converting " + filename)
         
    json_data["Channels"][ newName ] = json_data["Channels"][ oldName ]
    json_data["Channels"].pop( oldName )
    
    with open( filename, "w" ) as file:
        file.write( json.dumps( json_data, indent=4, sort_keys=True ) )

def ConvertFilesInDirectories( directory, oldName, newName ):
    filename = os.path.join( directory, "InsetChart.json" )
    if os.path.isfile( filename ):
        RenameChannelFromInsetChart( filename, oldName, newName )
    
    for obj in os.listdir( directory ):
        sub_dir = os.path.join( directory, obj )
        if os.path.isdir( sub_dir ):
            ConvertFilesInDirectories( sub_dir, oldName, newName )
        
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('directory', help='Directory InsetChart.json files or subdirectories that do.')
    args = parser.parse_args()
    
    ConvertFilesInDirectories( args.directory,
                               "Prevalence among Sexually Active (Adults)",
                               "Prevalence among Sexually Active" )

    