import threading
from cPlotBayesactSim import cPlotBayesactSim
from cEnum import eAxes, eEPA
import wx

class cOptionsPanel(wx.Panel):

    def __init__(self, parent, **kwargs):
        wx.Panel.__init__(self, parent, **kwargs)

        ## TODO: Pop elements out when windows are closed
        self.m_PlotFrames = []
        self.m_FocusedFrameTextBox = wx.TextCtrl(self, style=wx.TE_READONLY, pos=(200,200))
        self.m_FocusedFrame = None


    # Switches focus for separate plots
    def switchFocus(self, iPlotFrame):
        self.m_FocusedFrameTextBox.SetValue(iPlotFrame.GetLabel())
        self.m_FocusedFrame = iPlotFrame
