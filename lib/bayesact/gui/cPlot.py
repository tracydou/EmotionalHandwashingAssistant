import os
import signal
import threading
from cEnum import eAxes
from cConstants import cPlotConstants
import wx
import matplotlib.pyplot as plt


class cPlotFrame(wx.Frame):

    def __init__(self, iParent, **kwargs):
        wx.Frame.__init__(self, iParent, **kwargs)
        self.m_ParentOptionsPanel = iParent
        self.m_PlotPanel = None
        self.m_PlotBayesactSim = None
        self.m_PlotBayesactSimThread = None

        self.Bind(wx.EVT_CLOSE, self.onClose)

    def onClose(self, iEvent):
        if (None != self.m_PlotBayesactSim):
            self.m_PlotBayesactSim.sleepProcess()
            self.m_PlotBayesactSim.m_KeepAlive = False
            #self.m_PlotBayesactSimThread.join()
        iEvent.Skip()

    def initPanel(self, **kwargs):
        self.m_PlotPanel = cPlotPanel(self, **kwargs)


class cPlotPanel(wx.Panel):

    def __init__(self, iParent, **kwargs):
        wx.Panel.__init__(self, iParent, **kwargs)
        self.m_ParentPlotFrame = iParent

        self.m_Lock = threading.Lock()
        self.m_NumAxes = 3

        # Locks up axis when panning
        self.m_LockAxes = [False, False, False]

        self.m_PreviousMouseX = 0
        self.m_PreviousMouseY = 0

        self.m_XAxisMin = cPlotConstants.m_DefaultXAxisMin
        self.m_XAxisMax = cPlotConstants.m_DefaultXAxisMax
        self.m_YAxisMin = cPlotConstants.m_DefaultYAxisMin
        self.m_YAxisMax = cPlotConstants.m_DefaultYAxisMax
        self.m_ZAxisMin = cPlotConstants.m_DefaultZAxisMin
        self.m_ZAxisMax = cPlotConstants.m_DefaultZAxisMax

        self.m_XAxisLength = self.m_XAxisMax - self.m_XAxisMin
        self.m_YAxisLength = self.m_YAxisMax - self.m_YAxisMin
        self.m_ZAxisLength = self.m_ZAxisMax - self.m_ZAxisMin

        self.m_Figure = None
        self.m_Axes = None
        self.m_Canvas = None

        self.m_XData = None
        self.m_YData = None
        self.m_ZData = None

        # For blitting
        self.m_Background = None
        self.m_Patches = None

        self.SetBackgroundColour(cPlotConstants.m_BackgroundColour)

        # For resizing window
        self.Bind(wx.EVT_SIZE, self.onSize)
        self.m_n = 0

    def plotScatter(self, iXData, iYData, iAutoScaling=False, iRedraw=False, iUpdate=True, **kwargs):
        print 1
        pass


    def resetAxes(self):
        pass


    def updateAxesData(self):
        pass


    # modified from mpl.toolkits.mplot3d.axes3d._on_move
    def onMouseMove(self, iEvent):
        pass


    def onMousePress(self, iEvent):
        if (iEvent.inaxes == self.m_Axes):
            self.m_PreviousMouseX, self.m_PreviousMouseY = iEvent.xdata, iEvent.ydata


    def onMouseRelease(self, iEvent):
        # To be used with cOptions2D, else disable this functionality
        if not(isinstance(self.m_ParentPlotFrame, cPlotFrame)):
            return

        if (None != self.m_ParentPlotFrame.m_ParentOptionsPanel):
            self.m_ParentPlotFrame.m_ParentOptionsPanel.switchFocus(self.m_ParentPlotFrame)


    def onKeyPress(self, iEvent):
        self.updateAxesData()
        if ("{}".format(iEvent.key) == cPlotConstants.m_ZoomKey):
            self.zoomAxes(cPlotConstants.m_KeyZoomSensitivity)

        elif ("{}".format(iEvent.key) == cPlotConstants.m_UnZoomKey):
            self.zoomAxes(-cPlotConstants.m_KeyZoomSensitivity)

        elif ("{}".format(iEvent.key) == cPlotConstants.m_ResetAxesKey):
            self.resetAxes()

        elif ("{}".format(iEvent.key) == cPlotConstants.m_IncreaseXAxisKey):
            self.shiftXAxis(cPlotConstants.m_ShiftAxesSensitivity * (self.m_XAxisMax - self.m_XAxisMin))

        elif ("{}".format(iEvent.key) == cPlotConstants.m_DecreaseXAxisKey):
            self.shiftXAxis(-cPlotConstants.m_ShiftAxesSensitivity * (self.m_XAxisMax - self.m_XAxisMin))

        elif ("{}".format(iEvent.key) == cPlotConstants.m_IncreaseYAxisKey):
            self.shiftYAxis(cPlotConstants.m_ShiftAxesSensitivity * (self.m_YAxisMax - self.m_YAxisMin))

        elif ("{}".format(iEvent.key) == cPlotConstants.m_DecreaseYAxisKey):
            self.shiftYAxis(-cPlotConstants.m_ShiftAxesSensitivity * (self.m_YAxisMax - self.m_YAxisMin))

        elif ("{}".format(iEvent.key) == cPlotConstants.m_IncreaseZAxisKey):
            self.shiftZAxis(cPlotConstants.m_ShiftAxesSensitivity * (self.m_ZAxisMax - self.m_ZAxisMin))

        elif ("{}".format(iEvent.key) == cPlotConstants.m_DecreaseZAxisKey):
            self.shiftZAxis(-cPlotConstants.m_ShiftAxesSensitivity * (self.m_ZAxisMax - self.m_ZAxisMin))


    # modified from mpl.toolkits.mplot3d.axes3d._on_move
    def onScroll(self, iEvent):
        # "up" means scroll up
        if ("up" == iEvent.button):
            diffMouseScroll = cPlotConstants.m_ScrollSensitivity
        else:
            diffMouseScroll = -cPlotConstants.m_ScrollSensitivity

        self.zoomAxes(diffMouseScroll)


    def onSize(self, iEvent):
        pix = self.GetClientSize()
        self.m_Figure.set_size_inches(pix[0]/self.m_Figure.get_dpi(), pix[1]/self.m_Figure.get_dpi())
        x,y = self.GetSize()
        self.m_Canvas.SetSize((x-1, y-1))
        self.m_Canvas.SetSize((x, y))
        self.redrawCanvas()
        iEvent.Skip()


    def shiftXAxis(self, iShiftAmount):
        self.m_Axes.set_xlim([self.m_XAxisMin + iShiftAmount, self.m_XAxisMax + iShiftAmount])
        self.redrawAxes()


    def shiftYAxis(self, iShiftAmount):
        self.m_Axes.set_ylim([self.m_YAxisMin + iShiftAmount, self.m_YAxisMax + iShiftAmount])
        self.redrawAxes()


    def shiftZAxis(self, iShiftAmount):
        pass


    def zoomAxes(self, iZoomAmount):
        pass



    def addPatch(self, iPatch):
        self.m_Axes.add_patch(iPatch)
        #self.m_Axes.draw_artist(iPatch)

    # This thing doesn't seem to work right now
    def redrawPlot(self):
        print self.m_n
        self.m_n += 1
        with self.m_Lock:
            self.m_Axes.draw_artist(self.m_Axes.patch)
            #self.m_Axes.draw_artist(iPlot)
            self.m_Canvas.Update()
            #self.m_Canvas.Show()
            #self.m_Canvas.draw()
            self.m_Canvas.flush_events()


    def redrawAxes(self):
        # This thing will draw really really slowly if you are parsing and plotting at the same time
        # Parsing as in parsing it from a pipe
        with self.m_Lock:
            #self.m_Canvas.restore_region(self.m_Background)
            #self.m_Axes.draw_artist(self.m_Axes.patch)
            #self.m_Canvas.blit()
            self.m_Canvas.draw()
        #self.m_Background = [self.m_Canvas.copy_from_bbox(self.m_Axes.bbox)]
        #print self.m_Background


    # Remember to call redrawAxes
    def clearAxes(self):
        with self.m_Lock:
            self.m_Axes.cla()
