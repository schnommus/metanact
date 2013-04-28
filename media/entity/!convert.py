import json

inFileName = "scrap.edef"
outFileName = "scrap.json"

inFile = open(inFileName, "r")
outFile = open(outFileName, "w")

root = {}

for parameters in [l.strip().split() for l in inFile.readlines()]:
    if( parameters[0] == 'NAME'): root['Name'] = parameters[1]
    elif( parameters[0] == 'IMAGE'): root['Image'] = parameters[1]
    elif( parameters[0] == 'INHERITED_HEADING_VELOCITY'): root['InheritedHeadingVelocity'] = { 'amount': float(parameters[1]) }
    elif( parameters[0] == 'AFFECTED_BY_GRAVITY'): root['AffectedByGravity'] = { 'amount': float(parameters[1]) }
    elif( parameters[0] == 'SOUND_ON_CREATE'): root['SoundOnCreate'] = { 'filename': parameters[1] }
    elif( parameters[0] == 'VELOCITY_INHERITANCE'): root['VelocityInheritance'] = { 'factor': float(parameters[1]) }
    elif( parameters[0] == 'HAS_VELOCITY'): root['HasVelocity'] = {}
    elif( parameters[0] == 'VELOCITY_DETERMINES_HEADING'): root['VelocityDeterminesHeading'] = {}
    elif( parameters[0] == 'DECAYS'): root['VelocityDeterminesHeading'] = { 'startingAlpha': float(parameters[1]), 'startingScale': float(parameters[2]), 'finalAlpha': float(parameters[3]), 'finalScale': float(parameters[4]), 'deltaAlpha': float(parameters[5]), 'deltaScale': float(parameters[6]),  }
    elif( parameters[0] == 'HURT_TYPE_ON_RADIUS'):
        if( 'HurtTypeOnRadius' not in root ): ## Facilitates more than one of these tags
            root['HurtTypeOnRadius'] = [{ 'type':parameters[1], 'amount':float(parameters[2]), 'radius':float(parameters[3])}]
        else:
            root['HurtTypeOnRadius'] += [{ 'type':parameters[1], 'amount':float(parameters[2]), 'radius':float(parameters[3])}]
    elif( parameters[0] == 'IS_PERSISTANT'): root['IsPersistant'] = {}
    elif( parameters[0] == 'OVERRIDE_DRAW'): root['OverrideDraw'] = {}
    else: root[parameters[0]] = {}

outFile.write( json.dumps(root, sort_keys=False, indent=4, separators=(',', ': ')) )


inFile.close()
outFile.close()
