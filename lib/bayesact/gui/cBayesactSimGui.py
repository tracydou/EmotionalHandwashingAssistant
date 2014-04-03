import sys
sys.path.append("../")
from cConstants import cOptionSimConstants, cBayesactSimConstants, cSliderConstants, cEPAConstants
from cEnum import eEPA
import threading
import wx
import cPlotEPA2D
from cPlotBayesactThread import cPlotBayesactThread
from cNumericValidatorTextBox import cNumericValidatorTextBox


class cBayesactSimGuiPanel(wx.Panel):
    # The parent here is the cGuiTabs, which holds the gui itself and the options too
    def __init__(self, parent, iBayesactSim, iOptionsBayesactPanel, iBayesactInteractiveGuiPanel=None, **kwargs):
        wx.Panel.__init__(self, parent, **kwargs)

        self.m_OptionsBayesactSimPanel = iOptionsBayesactPanel
        self.m_BayesactSim = iBayesactSim
        self.m_BayesactInteractiveGuiPanel = iBayesactInteractiveGuiPanel
        self.m_Plotter = None

        self.m_BayesactSimThread = None



        # These are for all the options you can fill into the simulation
        ########################################################################################
        #self.m_SliderSize = (180, 24)
        #self.m_TextBoxSize = (110, 28)
        #self.m_ComboBoxSize = (110, 28)
        #self.m_ButtonSize = (190, 28)
        self.m_SliderSize = wx.DefaultSize
        self.m_TextBoxSize = wx.DefaultSize
        self.m_ComboBoxSize = wx.DefaultSize
        self.m_ButtonSize = (110, 30)


        # The Client and Agent alpha and beta values
        # Box sizes for windows and mac are: (110 by 28) and (106 by 20) pixels respectively
        self.m_ClientAlphaStaticText = wx.StaticText(self, -1, cOptionSimConstants.m_ClientAlpha, pos=(10,30))
        self.m_ClientAlphaTextBox = wx.TextCtrl(self, -1, pos=(170, 28), size=self.m_TextBoxSize,
                                                value=str(cOptionSimConstants.m_ClientAlphaDefault),
                                                validator=cNumericValidatorTextBox(iDecimals=True, iNegative=False))
        self.m_ClientAlphaTextBox.Bind(wx.EVT_TEXT, self.onSetClientAlphaFlag)
        # Default size on windows is (100, 24)
        self.m_ClientAlphaSlider = wx.Slider(self, -1, value=cOptionSimConstants.m_ClientAlphaDefault*cSliderConstants.m_Precision,
                                             pos=(290, 28), size=self.m_SliderSize,
                                             minValue=cOptionSimConstants.m_MinAlpha*cSliderConstants.m_Precision,
                                             maxValue=cOptionSimConstants.m_MaxAlpha*cSliderConstants.m_Precision,
                                             style=wx.SL_HORIZONTAL)
        self.m_ClientAlphaSlider.Bind(wx.EVT_SCROLL, lambda event, textBox=self.m_ClientAlphaTextBox: self.onChangeValueViaSlider(event, textBox))



        self.m_ClientBetaOfClientStaticText = wx.StaticText(self, -1, cOptionSimConstants.m_ClientBetaOfClient, pos=(10, 60))
        self.m_ClientBetaOfClientTextBox = wx.TextCtrl(self, -1, pos=(170, 58), size=self.m_TextBoxSize,
                                                       value=str(cOptionSimConstants.m_ClientBetaOfClientDefault),
                                                       validator=cNumericValidatorTextBox(iDecimals=True, iNegative=False))
        self.m_ClientBetaOfClientTextBox.Bind(wx.EVT_TEXT, self.onSetClientBetaOfClientFlag)

        self.m_ClientBetaOfClientSlider = wx.Slider(self, -1, value=cOptionSimConstants.m_ClientBetaOfClientDefault*cSliderConstants.m_Precision,
                                                    pos=(290, 58), size=self.m_SliderSize,
                                                    minValue=cOptionSimConstants.m_MinBeta*cSliderConstants.m_Precision,
                                                    maxValue=cOptionSimConstants.m_MaxBeta*cSliderConstants.m_Precision,
                                                    style=wx.SL_HORIZONTAL)
        self.m_ClientBetaOfClientSlider.Bind(wx.EVT_SCROLL, lambda event, textBox=self.m_ClientBetaOfClientTextBox: self.onChangeValueViaSlider(event, textBox))



        self.m_ClientBetaOfAgentStaticText = wx.StaticText(self, -1, cOptionSimConstants.m_ClientBetaOfAgent, pos=(10, 90))
        self.m_ClientBetaOfAgentTextBox = wx.TextCtrl(self, -1, pos=(170, 88), size=self.m_TextBoxSize,
                                                      value=str(cOptionSimConstants.m_ClientBetaOfAgentDefault),
                                                      validator=cNumericValidatorTextBox(iDecimals=True, iNegative=False))
        self.m_ClientBetaOfAgentTextBox.Bind(wx.EVT_TEXT, self.onSetClientBetaOfAgentFlag)

        self.m_ClientBetaOfAgentSlider = wx.Slider(self, -1, value=cOptionSimConstants.m_ClientBetaOfAgentDefault*cSliderConstants.m_Precision,
                                                   pos=(290, 88), size=self.m_SliderSize,
                                                   minValue=cOptionSimConstants.m_MinBeta*cSliderConstants.m_Precision,
                                                   maxValue=cOptionSimConstants.m_MaxBeta*cSliderConstants.m_Precision,
                                                   style=wx.SL_HORIZONTAL)
        self.m_ClientBetaOfAgentSlider.Bind(wx.EVT_SCROLL, lambda event, textBox=self.m_ClientBetaOfAgentTextBox: self.onChangeValueViaSlider(event, textBox))





        self.m_AgentAlphaStaticText = wx.StaticText(self, -1, cOptionSimConstants.m_AgentAlpha, pos=(10, 120))
        self.m_AgentAlphaTextBox = wx.TextCtrl(self, -1, pos=(170, 118), size=self.m_TextBoxSize,
                                               value=str(cOptionSimConstants.m_AgentAlphaDefault),
                                               validator=cNumericValidatorTextBox(iDecimals=True, iNegative=False))
        self.m_AgentAlphaTextBox.Bind(wx.EVT_TEXT, self.onSetAgentAlphaFlag)

        self.m_AgentAlphaSlider = wx.Slider(self, -1, value=cOptionSimConstants.m_AgentAlphaDefault*cSliderConstants.m_Precision,
                                            pos=(290, 118), size=self.m_SliderSize,
                                            minValue=cOptionSimConstants.m_MinAlpha*cSliderConstants.m_Precision,
                                            maxValue=cOptionSimConstants.m_MaxAlpha*cSliderConstants.m_Precision,
                                            style=wx.SL_HORIZONTAL)
        self.m_AgentAlphaSlider.Bind(wx.EVT_SCROLL, lambda event, textBox=self.m_AgentAlphaTextBox: self.onChangeValueViaSlider(event, textBox))







        self.m_AgentBetaOfClientStaticText = wx.StaticText(self, -1, cOptionSimConstants.m_AgentBetaOfClient, pos=(10, 150))
        self.m_AgentBetaOfClientTextBox = wx.TextCtrl(self, -1, pos=(170, 148), size=self.m_TextBoxSize,
                                                      value=str(cOptionSimConstants.m_AgentBetaOfClientDefault),
                                                      validator=cNumericValidatorTextBox(iDecimals=True, iNegative=False))
        self.m_AgentBetaOfClientTextBox.Bind(wx.EVT_TEXT, self.onSetAgentBetaOfClientFlag)

        self.m_AgentBetaOfClientSlider = wx.Slider(self, -1, value=cOptionSimConstants.m_AgentBetaOfClientDefault*cSliderConstants.m_Precision,
                                                   pos=(290, 148), size=self.m_SliderSize,
                                                   minValue=cOptionSimConstants.m_MinBeta*cSliderConstants.m_Precision,
                                                   maxValue=cOptionSimConstants.m_MaxBeta*cSliderConstants.m_Precision,
                                                   style=wx.SL_HORIZONTAL)
        self.m_AgentBetaOfClientSlider.Bind(wx.EVT_SCROLL, lambda event, textBox=self.m_AgentBetaOfClientTextBox: self.onChangeValueViaSlider(event, textBox))



        self.m_AgentBetaOfAgentStaticText = wx.StaticText(self, -1, cOptionSimConstants.m_AgentBetaOfAgent, pos=(10, 180))
        self.m_AgentBetaOfAgentTextBox = wx.TextCtrl(self, -1, pos=(170, 178), size=self.m_TextBoxSize,
                                                     value=str(cOptionSimConstants.m_AgentBetaOfAgentDefault),
                                                     validator=cNumericValidatorTextBox(iDecimals=True, iNegative=False))
        self.m_AgentBetaOfAgentTextBox.Bind(wx.EVT_TEXT, self.onSetAgentBetaOfAgentFlag)

        self.m_AgentBetaOfAgentSlider = wx.Slider(self, -1, value=cOptionSimConstants.m_AgentBetaOfAgentDefault*cSliderConstants.m_Precision,
                                                  pos=(290, 178), size=self.m_SliderSize,
                                                  minValue=cOptionSimConstants.m_MinBeta*cSliderConstants.m_Precision,
                                                  maxValue=cOptionSimConstants.m_MaxBeta*cSliderConstants.m_Precision,
                                                 style=wx.SL_HORIZONTAL)
        self.m_AgentBetaOfAgentSlider.Bind(wx.EVT_SCROLL, lambda event, textBox=self.m_AgentBetaOfAgentTextBox: self.onChangeValueViaSlider(event, textBox))






        self.m_GammaValueStaticText = wx.StaticText(self, -1, cOptionSimConstants.m_GammaValue, pos=(10, 210))
        self.m_GammaValueTextBox = wx.TextCtrl(self, -1, pos=(170, 208), size=self.m_TextBoxSize,
                                               value=str(cOptionSimConstants.m_GammaValueDefault),
                                               validator=cNumericValidatorTextBox(iDecimals=True, iNegative=True))
        self.m_GammaValueTextBox.Bind(wx.EVT_TEXT, self.onSetGammaValueFlag)

        self.m_GammaValueSlider = wx.Slider(self, -1, value=cOptionSimConstants.m_GammaValueDefault*cSliderConstants.m_Precision,
                                            pos=(290, 208), size=self.m_SliderSize,
                                            minValue=cOptionSimConstants.m_MinGamma*cSliderConstants.m_Precision,
                                            maxValue=cOptionSimConstants.m_MaxGamma*cSliderConstants.m_Precision,
                                            style=wx.SL_HORIZONTAL)
        self.m_GammaValueSlider.Bind(wx.EVT_SCROLL, lambda event, textBox=self.m_GammaValueTextBox: self.onChangeValueViaSlider(event, textBox))





        self.m_EnvironmentNoiseStaticText = wx.StaticText(self, -1, cOptionSimConstants.m_EnvironmentNoise, pos=(10, 240))
        self.m_EnvironmentNoiseTextBox = wx.TextCtrl(self, -1, pos=(170, 238), size=self.m_TextBoxSize,
                                                     value=str(cOptionSimConstants.m_EnvironmentNoiseDefault),
                                                     validator=cNumericValidatorTextBox(iDecimals=True, iNegative=True))
        self.m_EnvironmentNoiseTextBox.Bind(wx.EVT_TEXT, self.onSetEnvironmentNoiseFlag)

        self.m_EnvironmentNoiseSlider = wx.Slider(self, -1, value=cOptionSimConstants.m_EnvironmentNoiseDefault*cSliderConstants.m_Precision,
                                                  pos=(290, 238), size=self.m_SliderSize,
                                                  minValue=cOptionSimConstants.m_MinEnvironmentNoise*cSliderConstants.m_Precision,
                                                  maxValue=cOptionSimConstants.m_MaxEnvironmentNoise*cSliderConstants.m_Precision,
                                                  style=wx.SL_HORIZONTAL)
        self.m_EnvironmentNoiseSlider.Bind(wx.EVT_SCROLL, lambda event, textBox=self.m_EnvironmentNoiseTextBox: self.onChangeValueViaSlider(event, textBox))


        self.m_UniformDrawsStaticText = wx.StaticText(self, -1, cOptionSimConstants.m_UniformDraws, pos=(10, 270))
        self.m_UniformDrawsChoice = wx.ComboBox(self, -1, pos=(170, 268), size=self.m_ComboBoxSize,
                                                choices=cOptionSimConstants.m_UniformDrawsChoices,
                                                style=wx.CHOICEDLG_STYLE)
        self.m_UniformDrawsChoice.SetStringSelection(cOptionSimConstants.m_UniformDrawsDefault)
        self.m_UniformDrawsChoice.Bind(wx.EVT_COMBOBOX, self.onSetUniformDrawsFlag)


        self.m_NumberOfStepsStaticText = wx.StaticText(self, -1, cOptionSimConstants.m_NumSteps, pos=(10, 300))
        self.m_NumberOfStepsTextBox = wx.TextCtrl(self, -1, pos=(170, 298), size=self.m_TextBoxSize,
                                                  value=str(cOptionSimConstants.m_NumStepsDefault),
                                                  validator=cNumericValidatorTextBox(iDecimals=False, iNegative=False))
        self.m_NumberOfStepsTextBox.Bind(wx.EVT_TEXT, self.onSetNumStepsFlag)

        self.m_CurrentIterationsStaticText = wx.StaticText(self, -1, "Current Iterations", pos=(10, 330))
        self.m_CurrentIterationsTextBox = wx.TextCtrl(self, -1, pos=(170, 328), size=self.m_TextBoxSize, style=wx.TE_READONLY)
        self.m_CurrentIterationsTextBox.SetValue(str(self.m_BayesactSim.total_iterations))

        self.m_PreviousIterationsStaticText = wx.StaticText(self, -1, "Previous Iterations", pos=(10, 360))
        self.m_PreviousIterationsTextBox = wx.TextCtrl(self, -1, pos=(170, 358), size=self.m_TextBoxSize, style=wx.TE_READONLY)
        self.m_PreviousIterationsTextBox.SetValue("0")


        ########################################################################################

        # TODO: Both the SimGui and InteractiveGui share the same graph, currently I draw these 2 times each
        # Will need to have the tabs share the same Panel rather than the same PlotPanel, that is, put the two plot panels on one panel
        self.m_PlotEPAPanel2D_A = cPlotEPA2D.cPlotPanel(self,
                                                        iXAxisItem=cOptionSimConstants.m_XAxisDefaultKey,
                                                        iYAxisItem=cOptionSimConstants.m_YAxisDefaultKey,
                                                        iPlotType=eEPA.fundamental,
                                                        pos=(450, 0), size=(500, 275))

        self.m_PlotEPAPanel2D_B = cPlotEPA2D.cPlotPanel(self,
                                                        iXAxisItem=cOptionSimConstants.m_XAxisDefaultKey,
                                                        iYAxisItem=cOptionSimConstants.m_YAxisDefaultKey,
                                                        iPlotType=eEPA.tau,
                                                        pos=(450, 325), size=(500, 275))

        self.m_ChangeXAxisStaticText = wx.StaticText(self, -1, "X Axis", pos=(530, 290))
        self.m_ChangeXAxisComboBox = wx.ComboBox(self, -1, value=cOptionSimConstants.m_XAxisDefault, pos=(580, 285),
                                                 size=self.m_ComboBoxSize, choices=cEPAConstants.m_EPALabels)
        self.m_ChangeXAxisComboBox.Bind(wx.EVT_COMBOBOX, self.onChangeXAxis)

        self.m_ChangeXAxisStaticText = wx.StaticText(self, -1, "Y Axis", pos=(710, 290), size=self.m_ChangeXAxisStaticText.GetSize())
        self.m_ChangeYAxisComboBox = wx.ComboBox(self, -1, value=cOptionSimConstants.m_YAxisDefault, pos=(760, 285),
                                                 size=self.m_ComboBoxSize, choices=cEPAConstants.m_EPALabels)
        self.m_ChangeYAxisComboBox.Bind(wx.EVT_COMBOBOX, self.onChangeYAxis)

        self.m_StartButton = wx.Button(self, -1, label="Start", pos=(10, 410), size=self.m_ButtonSize)
        self.m_StartButton.Bind(wx.EVT_BUTTON, self.onStartBayesactSim)

        self.m_StepButton = wx.Button(self, -1, label="Step", pos=(10, 440), size=self.m_ButtonSize)
        self.m_StepButton.Bind(wx.EVT_BUTTON, self.onStepBayesactSim)

        self.m_StopButton = wx.Button(self, -1, label="Stop", pos=(10, 470), size=self.m_ButtonSize)
        self.m_StopButton.Bind(wx.EVT_BUTTON, self.onStopBayesactSim)

        #self.m_PauseButton = wx.Button(self, -1, label="Pause", pos=(10, 400), size=(190, 28))
        #self.m_PauseButton.Bind(wx.EVT_BUTTON, self.onPauseBayesactSim)

        #self.m_ResumeButton = wx.Button(self, -1, label="Resume", pos=(10, 430), size=(190, 28))
        #self.m_ResumeButton.Bind(wx.EVT_BUTTON, self.onResumeBayesactSim)

        #self.m_StopButton = wx.Button(self, -1, label="Stop", pos=(10, 460), size=(190, 28))
        #self.m_StopButton.Bind(wx.EVT_BUTTON, self.onStopBayesactSim)


        self.m_Green1StaticText = wx.StaticText(self, -1, "Green:", pos=(10, 520))
        self.m_Green2StaticText = wx.StaticText(self, -1, "What client thinks of themselves", pos=(70, 520))

        self.m_Pink1StaticText = wx.StaticText(self, -1, "Pink:", pos=(10, 540))
        self.m_Pink2StaticText = wx.StaticText(self, -1, "What client thinks of agent", pos=(70, 540))

        self.m_Yellow1StaticText = wx.StaticText(self, -1, "Yellow:", pos=(10, 560))
        self.m_Yellow2StaticText = wx.StaticText(self, -1, "What agent thinks of themselves", pos=(70, 560))

        self.m_Blue1StaticText = wx.StaticText(self, -1, "Blue:", pos=(10, 580))
        self.m_Blue2StaticText = wx.StaticText(self, -1, "What agent thinks of client", pos=(70, 580))


        self.updateSettingsFromBayesact()


    def onValueChange(self, iEvent):
        print iEvent.GetEventObject().GetValue()


    def redrawAxes(self, iPlotEPAPanel):
        if (None != self.m_Plotter):
            self.m_Plotter.replotOnPanel(iPlotEPAPanel)

    # To set the values of the gui to the values in bayesact
    def updateSettingsFromBayesact(self):
        self.m_OptionsBayesactSimPanel.updateSettingsFromBayesact()


    # To set the values of bayesact to the values in the gui
    # Should only be used to initialize
    def updateBayesactFromSettings(self):
        self.m_BayesactSim.client_alpha_value = float(self.m_ClientAlphaTextBox.GetValue())

        self.m_BayesactSim.client_beta_value_of_client = float(self.m_ClientBetaOfClientTextBox.GetValue())
        self.m_BayesactSim.client_beta_value_of_agent = float(self.m_ClientBetaOfAgentTextBox.GetValue())

        self.m_BayesactSim.agent_alpha_value = float(self.m_AgentAlphaTextBox.GetValue())

        self.m_BayesactSim.agent_beta_value_of_client = float(self.m_AgentBetaOfClientTextBox.GetValue())
        self.m_BayesactSim.agent_beta_value_of_agent = float(self.m_AgentBetaOfAgentTextBox.GetValue())

        self.m_BayesactSim.uniform_draws = bool(self.m_UniformDrawsChoice.GetStringSelection())
        self.m_BayesactSim.env_noise = float(self.m_EnvironmentNoiseTextBox.GetValue())
        self.m_BayesactSim.gamma_value = float(self.m_GammaValueTextBox.GetValue())

        self.m_OptionsBayesactSimPanel.updateBayesactFromSettings()


    def disableStartingOptions(self):
        self.m_OptionsBayesactSimPanel.disableStartingOptions()

    def enableStartingOptions(self):
        self.m_OptionsBayesactSimPanel.enableStartingOptions()

    def onSetClientAlphaFlag(self, iEvent):
        self.m_BayesactSim.update_client_alpha = True

    def onSetClientBetaOfClientFlag(self, iEvent):
        self.m_BayesactSim.update_client_beta_of_client = True

    def onSetClientBetaOfAgentFlag(self, iEvent):
        self.m_BayesactSim.update_client_beta_of_agent = True

    def onSetAgentAlphaFlag(self, iEvent):
        self.m_BayesactSim.update_agent_alpha = True

    def onSetAgentBetaOfClientFlag(self, iEvent):
        self.m_BayesactSim.update_agent_beta_value_of_client = True

    def onSetAgentBetaOfAgentFlag(self, iEvent):
        self.m_BayesactSim.update_agent_beta_value_of_agent = True

    def onSetGammaValueFlag(self, iEvent):
        self.m_BayesactSim.update_gamma_value = True

    def onSetEnvironmentNoiseFlag(self, iEvent):
        self.m_BayesactSim.update_environment_noise = True

    def onSetUniformDrawsFlag(self, iEvent):
        self.m_BayesactSim.update_uniform_draws = True

    def onSetNumStepsFlag(self, iEvent):
        self.m_BayesactSim.update_num_steps = True


    def onChangeValueViaSlider(self, iEvent, iTextBox):
        iTextBox.SetValue(str(iEvent.GetEventObject().GetValue() / cSliderConstants.m_Precision))



    def onChangeXAxis(self, iEvent):
        value = iEvent.GetEventObject().GetValue()

        self.m_PlotEPAPanel2D_A.m_XAxisItem = cEPAConstants.m_EPALabels.index(value)
        self.m_PlotEPAPanel2D_B.m_XAxisItem = cEPAConstants.m_EPALabels.index(value)

        self.redrawAxes(self.m_PlotEPAPanel2D_A)
        self.redrawAxes(self.m_PlotEPAPanel2D_B)


    def onChangeYAxis(self, iEvent):
        value = iEvent.GetEventObject().GetValue()

        self.m_PlotEPAPanel2D_A.m_YAxisItem = cEPAConstants.m_EPALabels.index(value)
        self.m_PlotEPAPanel2D_B.m_YAxisItem = cEPAConstants.m_EPALabels.index(value)

        self.redrawAxes(self.m_PlotEPAPanel2D_A)
        self.redrawAxes(self.m_PlotEPAPanel2D_B)


    def onStepBayesactSim(self, iEvent):
        if (None != self.m_BayesactSimThread):
            self.m_BayesactSim.step_bayesact_sim = True
            self.m_BayesactSim.thread_event.set()
        else:
            self.m_BayesactSim.update_num_steps = False
            self.onStartBayesactSim(None)

            # Done so that we can safely start stepping if the number of steps is greater than 1 due a a race condition
            self.m_BayesactSim.start_sim_lock.acquire()

            #with self.m_BayesactSim.start_sim_lock:
            #    pass

            if (1 < int(self.m_NumberOfStepsTextBox.GetValue())):
                self.m_BayesactSim.num_steps = int(self.m_NumberOfStepsTextBox.GetValue()) - 1
                self.m_BayesactSim.step_bayesact_sim = True

                self.m_BayesactSim.acquire_stepping_lock = True
                self.m_BayesactSim.thread_event.set()

                # I wanted to set the update flag, but since it checks the flags, a race condition occurs
                # So I have to wait until it finishes checking the flag, and then either update the num steps or set the flag
                self.m_BayesactSim.stepping_lock.acquire()
                #with self.m_BayesactSim.stepping_lock:
                #    pass

                self.m_BayesactSim.num_steps = int(self.m_NumberOfStepsTextBox.GetValue())


    # Stops bayesact sim and kills thread
    def onStopBayesactSim(self, iEvent=None):
        if (None != self.m_BayesactSimThread):
            self.m_BayesactSim.terminate_flag = True
            self.m_BayesactSim.thread_event.set()
            self.m_BayesactSimThread.join()
            self.m_BayesactSimThread = None
            self.enableStartingOptions()


    def onStartBayesactSim(self, iEvent):
        #self.m_StartButton.SetLabel("Restart")
        self.onStopBayesactSim()

        self.disableStartingOptions()

        self.updateBayesactFromSettings()

        self.m_Plotter = cPlotBayesactThread()

        self.m_Plotter.initPlotBayesactSim(self.m_PlotEPAPanel2D_A)
        self.m_Plotter.addPanel(self.m_PlotEPAPanel2D_B)

        self.m_BayesactSim.plotter=self.m_Plotter
        self.m_BayesactSim.terminate_flag = False

        self.m_BayesactSimThread = threading.Thread(target=self.m_BayesactSim.startThread)

        self.m_BayesactSim.thread_event = threading.Event()

        self.m_Plotter.setThread(self.m_BayesactSimThread)
        self.m_Plotter.startThread()

        if ((None != self.m_BayesactInteractiveGuiPanel) and (None == self.m_BayesactInteractiveGuiPanel.m_BayesactSimThread)):
            self.m_BayesactInteractiveGuiPanel.m_BayesactSimThread = self.m_BayesactSimThread
            self.m_BayesactInteractiveGuiPanel.m_Plotter = self.m_Plotter
            self.m_Plotter.addPanel(self.m_BayesactInteractiveGuiPanel.m_PlotEPAPanel2D_A)
            self.m_Plotter.addPanel(self.m_BayesactInteractiveGuiPanel.m_PlotEPAPanel2D_B)



    # DEPRECATED
    def onPauseBayesactSim(self, iEvent=None):
        if (None != self.m_BayesactSimThread):
            # Queues up a pause event for the thread, the program will pause when it hits its thread_event.wait()
            self.m_BayesactSim.thread_event.clear()

    # DEPRECATED
    def onResumeBayesactSim(self, iEvent=None):
        if (None != self.m_BayesactSimThread):
            self.m_BayesactSim.thread_event.set()


