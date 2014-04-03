import threading
from cPlotBayesactSim import cPlotBayesactSim
from cEnum import eAxes, eEPA
from cOptions import cOptionsPanel
import wx
import cPlotEPA3D

class cOptionsPanel3D(cOptionsPanel):

    def __init__(self, iParent, **kwargs):
        cOptionsPanel.__init__(self, iParent, **kwargs)


        self.m_PlotButton3D = wx.Button(self, label="3D Make", size=wx.DefaultSize, pos=(10,10))
        self.m_PlotButton3D.Bind(wx.EVT_BUTTON, self.onMake3D)
        self.m_ScatterButton3D = wx.Button(self, label="Scatter 3D", size=wx.DefaultSize, pos=(10,80))
        self.m_ScatterButton3D.Bind(wx.EVT_BUTTON, self.onScatter3D)


        self.m_SleepThreadButton = wx.Button(self, label="Sleep Thread", size=wx.DefaultSize, pos=(10,290))
        self.m_SleepThreadButton.Bind(wx.EVT_BUTTON, self.onSleepThread)

        self.m_ContinueThreadButton = wx.Button(self, label="Continue Thread", size=wx.DefaultSize, pos=(10,360))
        self.m_ContinueThreadButton.Bind(wx.EVT_BUTTON, self.onContinueThread)

        self.m_PlotSim3DButton = wx.Button(self, label="BayesactSim3D", size=wx.DefaultSize, pos=(10,430))
        self.m_PlotSim3DButton.Bind(wx.EVT_BUTTON, self.onPlotBayesactSim3D)


        self.m_Clear3DButton = wx.Button(self, label="Clear3D", size=wx.DefaultSize, pos=(10,570))
        self.m_Clear3DButton.Bind(wx.EVT_BUTTON, self.onClear3D)

        #self.m_Slider1 = wx.Slider(self, pos=(100, 10), style=wx.SL_VERTICAL)

        #self.Bind(wx.EVT_CLOSE, self.onClose)

    def onClear3D(self, iEvent):
        self.m_FocusedFrame.m_PlotPanel.clearGraph()


    def onMake3D(self, iEvent):
        frame = cPlotEPA3D.cPlotFrame(self, title="Plot{}".format(len(self.m_PlotFrames) + 1), size=(700, 550))
        frame.initPanel(iXAxisItem=eEPA.evaluation,
                        iYAxisItem=eEPA.potency,
                        iZAxisItem=eEPA.activity,
                        style=wx.SIMPLE_BORDER, pos=(0, 0), size=(700, 550))
        frame.Show()
        #frame.m_PlotPanel.m_Axes.figure.canvas.draw()
        self.m_PlotFrames.append(frame)
        self.switchFocus(frame)


    def onPlotBayesactSim3D(self, iEvent):
        plotBayesactSim = cPlotBayesactSim(self.m_FocusedFrame.m_PlotPanel)

        self.m_FocusedFrame.m_PlotBayesactSim = plotBayesactSim
        self.m_FocusedFrame.m_PlotBayesactSimThread = threading.Thread(target=plotBayesactSim.runOnPlot)
        self.m_FocusedFrame.m_PlotBayesactSimThread.daemon = True
        self.m_FocusedFrame.m_PlotBayesactSimThread.start()


    def onSleepThread(self, iEvent):
        self.m_FocusedFrame.m_PlotBayesactSim.sleepProcess()


    def onContinueThread(self, iEvent):
        self.m_FocusedFrame.m_PlotBayesactSim.continueProcess()


    def onScatter3D(self, iEvent):
        self.m_FocusedFrame.m_PlotPanel.m_Axes.scatter([0, 1], [-2, 1], [4, 1], marker="o", s=50, c="goldenrod", alpha=1)
        #self.m_FocusedFrame.m_PlotPanel.plotScatter([0, 1], [-2, 1], [4, 1], iRedraw=True, marker="o", s=50, c="goldenrod", alpha=1)




class cDefaultFrame(wx.Frame):

    def __init__(self, iParent, **kwargs):
        wx.Frame.__init__(self, iParent, **kwargs)
        self.optionsPanel = cOptionsPanel3D(self, style=wx.SIMPLE_BORDER, size=self.GetSize())


def run(argv):
    app = wx.App(redirect=False)
    frame = cDefaultFrame(None, title="Title", size=(500, 700))
    frame.Show()
    app.MainLoop()
