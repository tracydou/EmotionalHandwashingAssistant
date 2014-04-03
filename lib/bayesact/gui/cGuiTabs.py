from cOptionsAgent import cOptionsAgentPanel
from cBayesactSimGui import cBayesactSimGuiPanel
from cBayesactInteractiveGui import cBayesactInteractiveGuiPanel
from cBayesactSim import cBayesactSim
from cBayesactInteractive import cBayesactInteractive
from cOptionsBayesactSim import cOptionsBayesactSimPanel
from cOptionsBayesactInteractive import cOptionsBayesactInteractivePanel
import cSimInteractiveTabs
import wx
import sys


class cDefaultFrame(wx.Frame):
    def __init__(self, parent, **kwargs):
        wx.Frame.__init__(self, parent, wx.ID_ANY, "Bayesact", size=(1100,720), **kwargs)

        self.m_Panel = wx.Panel(self)

        self.m_GuiTabs = cGuiTabs(self.m_Panel)
        self.m_Sizer = wx.BoxSizer(wx.VERTICAL)
        self.m_Sizer.Add(self.m_GuiTabs, 1, wx.ALL|wx.EXPAND, 5)
        self.m_Panel.SetSizer(self.m_Sizer)
        self.Layout()
        self.Show()


class cGuiTabs(wx.Notebook):
    def __init__(self, parent):
        wx.Notebook.__init__(self, parent, id=wx.ID_ANY, style=wx.BK_TOP)

        # Bayesact Sim
        self.m_BayesactSim = cBayesactSim()

        # Options agent tab
        self.m_OptionsAgentPanel = cOptionsAgentPanel(self)

        # Initial Settings tab for bayesact sim
        self.m_OptionsBayesactSimPanel = cOptionsBayesactSimPanel(self, self.m_BayesactSim, self.m_OptionsAgentPanel)

        # The interchangeable gui for both bayesactsim and bayesactinteractive
        self.m_SimInteractiveTabsPanel = cSimInteractiveTabs.cDefaultPanel(self, self.m_BayesactSim, self.m_OptionsAgentPanel, self.m_OptionsBayesactSimPanel)
        
        self.AddPage(self.m_OptionsAgentPanel, "Define Interactants")
        self.AddPage(self.m_OptionsBayesactSimPanel, "Bayesact Simulator Settings")
        self.AddPage(self.m_SimInteractiveTabsPanel, "Bayesact Simulator")

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



def main(argv):
    app = wx.App(redirect=False)
    frame = cDefaultFrame(None)
    app.MainLoop()

if __name__ == "__main__":
    main(sys.argv)
