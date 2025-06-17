##### import the simple module from the paraview
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()
import os

### WARNING: Not tested since a while :WARNING ###

DATAPATH=os.environ["VERROU_PARAVIEW_DATA_PATH"]

try:
  DATAPATH
except NameError:
  print("Please set the DATAPATH variable with the full path to the directory that contains this script and the CSV files. Type: DATAPATH='/path/to/the_cvs_datasets/'")

RemoveViewsAndLayouts()

layout1 = CreateLayout()

pathTime=DATAPATH+'/paraviewTime.csv'
pathParam=DATAPATH+'/paraviewParam.csv'

# create a new 'CSV Reader'
timeCsv = CSVReader(FileName=[pathTime], FieldDelimiterCharacters="\t")
paramCsv = CSVReader(FileName=[pathParam], FieldDelimiterCharacters="\t")

keyInput=((open(pathParam)).readline().strip()).split("\t")

#rangeInput=[ line.split("\t")[0] for line in open(pathParam).readlines()] 


timeIndex=((open(pathTime)).readline().strip()).split("\t")[1:]
keyIndex=[(line.split("\t"))[0] for line in ((open(pathTime)).readlines())][1:]



transposeTable1 = TransposeTable(Input=timeCsv)
transposeTable1.VariablesofInterest=timeIndex
transposeTable1.Addacolumnwithoriginalcolumnsname = 0




lineChartView1=CreateView("XYFunctionalBagChartView")

lineDisplay= Show(transposeTable1, lineChartView1)

lineDisplay.AttributeType= 'Row Data'

rangeIn=[x for x in  lineDisplay.SeriesLabel ]
nbSample=len(keyIndex)
if len(rangeIn)!=2*nbSample:
  print("Incoherent size")
  sys.exit()
for i in range(nbSample):
  rangeIn[2*i+1]=keyIndex[i]



lineDisplay.SeriesLabel= rangeIn
lineDisplay.SeriesVisibility= rangeIn

SetActiveSource(transposeTable1)

layout1.SplitVertical(0, 0.5)
# Create a new 'Parallel Coordinates View'
parallelCoordinatesView1 = CreateView('ParallelCoordinatesChartView')

input_parameterscsvDisplay = Show(paramCsv, parallelCoordinatesView1)

# trace defaults for the display properties.
input_parameterscsvDisplay.CompositeDataSetIndex = 0
input_parameterscsvDisplay.FieldAssociation = 'Row Data'
input_parameterscsvDisplay.SeriesVisibility = keyInput


# Properties modified on campbell_1d_input_parameterscsvDisplay
input_parameterscsvDisplay.SeriesVisibility = keyInput

#### uncomment the following to render all views
RenderAllViews()
