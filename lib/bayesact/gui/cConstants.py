from sys import platform as _platform
from cEnum import eGender, eEPA

class cWindowsOSConstants:
    m_ProcessTerminate = 1

class cSystemConstants:
    m_OS = 0

    m_LinuxOS = 0b0001
    m_MacOS = 0b0010
    m_WindowsOS = 0b0100

    if "linux" == _platform or "linux2" == _platform:
        m_OS |= 0b0001
    elif "darwin" == _platform:
        m_OS |= 0b0010
    elif "win32" == _platform:
        m_OS |= 0b0100


class cColourConstants:
    m_Colours = ["blue", "green", "red", "pink", "cyan", "magenta",
                 "yellow", "goldenrod", "black", "white"]


class cEPAConstants:
    # Try not to change the ordering of this, as cEnum.eEPA depends on this
    m_EPALabels = ["Evaluation", "Potency", "Activity"]
    m_Dimensions = 3
    m_SelfMultiplier = 0
    m_ActionMultiplier = 1
    m_OtherMultiplier = 2

    m_NumAttributes = 9


class cPlotConstants:
    # Adjust how many samples of each agent to plot
    m_MaxPlotSamples = 100

    m_ZoomKey = "ctrl+="
    m_UnZoomKey = "ctrl+-"
    m_ResetAxesKey = "ctrl+d"

    m_IncreaseXAxisKey = "ctrl+w"
    m_DecreaseXAxisKey = "ctrl+q"
    m_IncreaseYAxisKey = "ctrl+a"
    m_DecreaseYAxisKey = "ctrl+s"
    m_IncreaseZAxisKey = "ctrl+x"
    m_DecreaseZAxisKey = "ctrl+z"

    # 3 represents right click
    m_MousePanButton = 3

    m_DefaultXAxisMin = -4.3
    m_DefaultXAxisMax = 4.3
    m_DefaultYAxisMin = -4.3
    m_DefaultYAxisMax = 4.3
    m_DefaultZAxisMin = -4.3
    m_DefaultZAxisMax = 4.3

    m_BackgroundColour = "WHITE"

    m_ShiftAxesSensitivity = 0.1
    m_ScrollSensitivity = 1.0
    m_KeyZoomSensitivity = 2.0


class cPlot2DConstants:
    m_MouseDragSensitivity = 1
    m_PanSensitivity = 1.0 / (m_MouseDragSensitivity * 100)

    # This is for getting the dpi for figsize
    # X is taken from (400 / 100) * 1.2
    # Y is taken from (300 / 100) * 1.2
    m_FigRatioX = 0.012
    m_FigRatioY = 0.012

    # Used for adjusting the position and size of the graph is on the panel
    # Left, Bottom, Width, Height, all of which are fractions of figure width and height or the panel in the range [0,1]
    # These values were largely chosen for keeping the x and y axis labels visible
    m_Rect = [0.19, 0.15, 0.8, 0.8]


class cPlot3DConstants:
    # 1 represents left click
    m_MouseRotateButton = 1

    m_DefaultElev = 30
    m_DefaultAzim = -60

    m_MouseDragSensitivity = 10.0
    m_PanSensitivity = 1.0 / (m_MouseDragSensitivity * 10)


class cParseConstants:
    m_Command = "python bayesactsim.py -v"

    m_IterationPhrase = "-d-d-d-d-d-d-d-d-d-d iter"
    m_SamplesPhrase = "!!!!!! unweighted set:"
    m_MeanLearnerPhrase1 = "learner f is:"
    m_MeanLearnerPhrase2 = "learner average f:"
    m_MeanSimulatorPhrase1 = "simulator f is:"
    m_MeanSimulatorPhrase2 = "simulator average:"
    m_fValuesPhrase = "^f : \["


class cDataFilesConstants:
    m_fidentities = "fidentities.dat"


# Please keep the elements in the arrays in the same order as their name and key counterparts
class cInstitutionsConstants:
    m_AnyGender          = "Any Gender"
    m_Male               = "Male"
    m_Female             = "Female"

    m_Gender             = ["Any", "Male", "Female"]

    m_Institution        = ["Any",
                            "Lay",
                            "Business",
                            "Law",
                            "Politics",
                            "Academe",
                            "Medicine",
                            "Religion",
                            "Family",
                            "Sexual"]

    m_WhatIsThis         = ["what",
                            "is",
                            "this",
                            "?"
                            ]

    m_GenderKey          = [0b11,
                            0b10,
                            0b01]

    m_InstitutionKey     = [0b111111111,
                            0b100000000,
                            0b010000000,
                            0b001000000,
                            0b000100000,
                            0b000010000,
                            0b000001000,
                            0b000000100,
                            0b000000010,
                            0b000000001]

    m_WhatIsThisKey      = [0b111,
                            0b001,
                            0b010,
                            0b100]


class cOptionsAgentConstants:
    m_ClientMultipleIdentity = False
    m_GenderChoices = ["Male", "Female"]

    m_AgentGenderDefault = m_GenderChoices[eGender.male]
    m_ClientGenderDefault = m_GenderChoices[eGender.male]



class cOptionSimConstants:
    #0: knows own id, not client id
    #1: knows own id, knows client id is one of num_confusers possibilities
    #2: knows own id, knows client id
    #3: doesn't know own id, doesn't know client id
    m_KnowledgeChoices = ["0", "1", "2", "3"]
    #m_KnowledgeChoices = ["0", "2", "3"]

    m_UniformDrawsChoices = ["True", "False"]

    # Agent and client id to be set in options

    m_NumberOfSamples                     = "Number of Samples"
    m_NumberOfTrials                      = "Number of Trials"
    m_NumberOfExperiments                 = "Number of Experiments"
    m_ClientKnowledge                     = "Client Knowledge"
    m_AgentKnowledge                      = "Agent Knowledge"
    m_MaxHorizon                          = "Max Horizon"
    m_UniformDraws                        = "Uniform Draws"
    m_RougheningNoise                     = "Roughening Noise"
    m_EnvironmentNoise                    = "Environment Noise"
    m_GammaValue                          = "Gamma Value"
    m_AgentGender                         = "Agent Gender"
    m_ClientGender                        = "Client Gender"

    m_ClientAlpha                         = "Client Alpha"

    m_ClientBetaOfClient                  = "Client Beta of Client"
    m_ClientBetaOfAgent                   = "Client Beta of Agent"

    m_AgentAlpha                          = "Agent Alpha"

    m_AgentBetaOfClient                   = "Agent Beta of Client"
    m_AgentBetaOfAgent                    = "Agent Beta of Agent"

    m_NumSteps                            = "Number of Steps"

    m_NumberOfSamplesKey                  = "-n"
    m_NumberOfTrialsKey                   = "-t"
    m_NumberOfExperimentsKey              = "-x"
    m_ClientKnowledgeKey                  = "-c"
    m_AgentKnowledgeKey                   = "-a"
    m_MaxHorizonKey                       = "-d"
    m_UniformDrawsKey                     = "-u"
    m_RougheningNoiseKey                  = "-r"
    m_EnvironmentNoiseKey                 = "-e"
    m_GammaValueKey                       = "-g"
    m_AgentGenderKey                      = "-k"
    m_ClientGenderKey                     = "-l"
    m_VerboseKey                          = "-v"

    m_NumberOfSamplesDefault              = 1000
    m_NumberOfTrialsDefault               = 20
    m_NumberOfExperimentsDefault          = 10
    m_ClientKnowledgeDefault              = m_KnowledgeChoices[0]
    m_AgentKnowledgeDefault               = m_KnowledgeChoices[0]
    m_MaxHorizonDefault                   = 50
    m_UniformDrawsDefault                 = "False"
    m_RougheningNoiseDefault              = -1.0
    m_EnvironmentNoiseDefault             = 0.0
    m_GammaValueDefault                   = 1.0

    m_ClientAlphaDefault                  = 1.0

    m_ClientBetaOfClientDefault           = 0.005
    m_ClientBetaOfAgentDefault            = 0.005

    m_AgentAlphaDefault                   = 1.0

    m_AgentBetaOfClientDefault            = 0.005
    m_AgentBetaOfAgentDefault             = 0.005

    m_NumStepsDefault                     = 1

    m_MinAlpha                            = 0.001
    m_MinBeta                             = 0.001
    m_MinGamma                            = 0.001
    m_MinEnvironmentNoise                 = 0.001

    m_MaxAlpha                            = 2.0
    m_MaxBeta                             = 2.0
    m_MaxGamma                            = 2.0
    m_MaxEnvironmentNoise                 = 2.0

    m_XAxisDefaultKey                     = eEPA.evaluation
    m_YAxisDefaultKey                     = eEPA.potency
    m_XAxisDefault                        = cEPAConstants.m_EPALabels[m_XAxisDefaultKey]
    m_YAxisDefault                        = cEPAConstants.m_EPALabels[m_YAxisDefaultKey]


class cBayesactSimConstants:
    m_BayesactSimFileName = "bayesactsim.py"

class cSliderConstants:
    # Logarithmic, since I don't know how to use decimal points in sliders,
    # I will just set the max value to be n times larger than usual, then divide it
    m_Precision = 1000.0
