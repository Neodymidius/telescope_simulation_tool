import xml.etree.ElementTree as ET
import sys

path = sys.argv[1]
type = sys.argv[2]
type_shadowing = sys.argv[3]
factor = sys.argv[4]
shadowing_factor = sys.argv[5]

tree = ET.parse(path)
root = tree.getroot()

for surface in root.iter('surface'):
    # Check for matching attributes or just edit directly
    surface.set('type', str(shadowing_factor))
    surface.set('shadowing', str(type_shadowing))
    surface.set('roughness', str(shadowing_factor))
    surface.set('shadowing_alpha', str(shadowing_factor))

tree.write(path)