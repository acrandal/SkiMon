

import fileinput


def printHeader():
    print("""<?xml version="1.0" encoding="UTF-8"?>""")
    print("""<kml xmlns="http://www.opengis.net/kml/2.2">""")
    print("""  <Document>""")
    print("""    <name>SkiMon Day 01</name>""")
    print("""    <description>
        2021.04.13 on Mountain such and such
        </description>
    """)

def printStyles():
    print("""
    <Style id="yellowLineGreenPoly">
      <LineStyle>
        <color>7f00ffff</color>
        <width>4</width>
      </LineStyle>
      <PolyStyle>
        <color>7f00ff00</color>
      </PolyStyle>
    </Style>
    
    """)


def printPlacemarks(coordinates):
    print("""
    <Placemark>
      <name>Absolute Extruded</name>
      <description>Transparent green wall with yellow outlines</description>
      <styleUrl>#yellowLineGreenPoly</styleUrl>
      <LineString>
        <extrude>1</extrude>
        <tessellate>1</tessellate>
        <altitudeMode>absolute</altitudeMode>
    """)
    print("""    <coordinates>""")

    for coord in coordinates:
        vals = [coord["longitude"], coord["latitude"], coord["altitude"] ]
        line = ",".join(vals)
        print(line)

          #-112.2550785337791,36.07954952145647,2357
    print("""   </coordinates>""")
    print("""
          </LineString>
        </Placemark>
    """)

def printFooter():
    print("""  </Document>
    </kml>""")


if __name__ == "__main__":
#    print("Generating KML")

    coordinates = [ ]

    for line in fileinput.input():
        line = line.strip()
        dat = line.split(',')

        if dat[0] == "stamp":
            continue

        newCoord = {}
        newCoord["latitude"] = dat[8]
        newCoord["longitude"] = dat[9]
        newCoord["altitude"] = dat[10]
        coordinates.append(newCoord)



    printHeader()
    printStyles()
    printPlacemarks(coordinates)
    printFooter()


#    print("Done")