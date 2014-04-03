import threading
from cPlotBayesactSim import cPlotBayesactSim
from cEnum import eAxes, eEPA
import wx
import cPlotEPA2D
from cOptions import cOptionsPanel

class cOptionsPanel2D(cOptionsPanel):

    def __init__(self, parent, **kwargs):
        cOptionsPanel.__init__(self, parent, **kwargs)

        self.m_PlotButton2D = wx.Button(self, label="2D Make", size=wx.DefaultSize, pos=(10,150))
        self.m_PlotButton2D.Bind(wx.EVT_BUTTON, self.onMake2D)

        self.m_SleepThreadButton = wx.Button(self, label="Sleep Thread", size=wx.DefaultSize, pos=(10,290))
        self.m_SleepThreadButton.Bind(wx.EVT_BUTTON, self.onSleepThread)

        self.m_ContinueThreadButton = wx.Button(self, label="Continue Thread", size=wx.DefaultSize, pos=(10,360))
        self.m_ContinueThreadButton.Bind(wx.EVT_BUTTON, self.onContinueThread)

        self.m_PlotSim2DButton = wx.Button(self, label="BayesactSim2D", size=wx.DefaultSize, pos=(10,500))
        self.m_PlotSim2DButton.Bind(wx.EVT_BUTTON, self.onPlotBayesactSim2D)


        #self.m_Slider1 = wx.Slider(self, pos=(100, 10), style=wx.SL_VERTICAL)

        #self.Bind(wx.EVT_CLOSE, self.onClose)



    def onPlotBayesactSim2D(self, iEvent):
        plotBayesactSim = cPlotBayesactSim(self.m_FocusedFrame.m_PlotPanel)
        self.m_FocusedFrame.m_PlotBayesactSim = plotBayesactSim
        self.m_FocusedFrame.m_PlotBayesactSimThread = threading.Thread(target=plotBayesactSim.runOnPlot)
        self.m_FocusedFrame.m_PlotBayesactSimThread.daemon = True
        self.m_FocusedFrame.m_PlotBayesactSimThread.start()

    def onSleepThread(self, iEvent):
        self.m_FocusedFrame.m_PlotBayesactSim.sleepProcess()


    def onContinueThread(self, iEvent):
        self.m_FocusedFrame.m_PlotBayesactSim.continueProcess()


    def onMake2D(self, iEvent):
        frame = cPlotEPA2D.cPlotFrame(self, title="Plot{}".format(len(self.m_PlotFrames) + 1), size=(800, 600))
        frame.initPanel(iXAxisItem=eEPA.evaluation,
                        iYAxisItem=eEPA.potency,
                        style=wx.SIMPLE_BORDER, pos=(0, 0), size=(800, 600))
        frame.Show()
        self.m_PlotFrames.append(frame)
        self.switchFocus(frame)



class cDefaultFrame(wx.Frame):

    def __init__(self, iParent, **kwargs):
        wx.Frame.__init__(self, iParent, **kwargs)
        self.optionsPanel = cOptionsPanel2D(self, style=wx.SIMPLE_BORDER, size=self.GetSize())


def run(argv):
    app = wx.App(redirect=False)
    frame = cDefaultFrame(None, title="Title", size=(500, 700))
    frame.Show()
    app.MainLoop()
