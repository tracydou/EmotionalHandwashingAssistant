from cOptionsAgent import cOptionsAgentPanel
from cBayesactSimGui import cBayesactSimGuiPanel
from cBayesactInteractiveGui import cBayesactInteractiveGuiPanel
import wx
import sys


class cDefaultPanel(wx.Panel):
    def __init__(self, parent, iBayesactSim, iOptionsAgentPanel, iOptionsBayesactPanel, **kwargs):
        wx.Panel.__init__(self, parent, **kwargs)

        self.m_SimInteractiveTabs = cSimInteractiveTabs(self, iBayesactSim, iOptionsAgentPanel, iOptionsBayesactPanel)
        self.m_Sizer = wx.BoxSizer(wx.VERTICAL)
        self.m_Sizer.Add(self.m_SimInteractiveTabs, 1, wx.ALL|wx.EXPAND, 5)
        self.SetSizer(self.m_Sizer)
        self.Layout()
        self.Show()

class cSimInteractiveTabs(wx.Notebook):
    def __init__(self, parent, iBayesactSim, iOptionsAgentPanel, iOptionsBayesactPanel):
        wx.Notebook.__init__(self, parent, id=wx.ID_ANY, style=wx.BK_LEFT)

        # bayesactsim tab
        self.m_BayesactSimGuiPanel = cBayesactSimGuiPanel(self, iBayesactSim, iOptionsBayesactPanel)

        # bayeact interactive tab
        self.m_BayesactInteractiveGuiPanel = cBayesactInteractiveGuiPanel(self, iBayesactSim, iOptionsBayesactPanel, self.m_BayesactSimGuiPanel)
        self.m_BayesactSimGuiPanel.m_BayesactInteractiveGuiPanel = self.m_BayesactInteractiveGuiPanel

        iBayesactSim.sim_gui = self.m_BayesactSimGuiPanel
        iBayesactSim.interactive_gui = self.m_BayesactInteractiveGuiPanel

        self.AddPage(self.m_BayesactSimGuiPanel, "Simulator")
        self.AddPage(self.m_BayesactInteractiveGuiPanel, "Interactive")

        self.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGED, self.OnPageChanged)
        self.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGING, self.OnPageChanging)

        # To get rid of an issue with a little black box appearing on the top left of the window in MSW
        self.SendSizeEvent()


    def OnPageChanged(self, event):
        #old = event.GetOldSelection()
        #new = event.GetSelection()
        #sel = self.GetSelection()
        #print 'OnPageChanged,  old:%d, new:%d, sel:%d\n' % (old, new, sel)
        event.Skip()

    def OnPageChanging(self, event):
        #old = event.GetOldSelection()
        #new = event.GetSelection()
        #sel = self.GetSelection()
        #print 'OnPageChanging, old:%d, new:%d, sel:%d\n' % (old, new, sel)
        event.Skip()


