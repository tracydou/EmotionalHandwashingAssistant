import wx
import wx.lib.agw.balloontip as cBT

def makeHoverBalloonTip(iWidget, iMessage):

    balloon = cBT.BalloonTip(topicon=None,
                             message=iMessage,
                             shape=cBT.BT_ROUNDED,
                             tipstyle=cBT.BT_LEAVE)
    # set the BalloonTip target
    balloon.SetTarget(iWidget)
    # set the BalloonTip background colour
    balloon.SetBalloonColour(wx.WHITE)
    # set the font for the balloon title
    #balloon.SetTitleFont(wx.Font(12, wx.SWISS, wx.NORMAL, wx.BOLD, False))
    # set the colour for the balloon title
    #balloon.SetTitleColour(wx.BLACK)
    # leave the message font as default
    balloon.SetMessageFont(wx.Font(12, wx.SWISS, wx.NORMAL, wx.NORMAL))
    # set the message (tip) foreground colour
    balloon.SetMessageColour(wx.BLACK)
    # set the start delay for the BalloonTip
    balloon.SetStartDelay(1)
    # set the time after which the BalloonTip is destroyed
    #balloon.SetEndDelay(3000)

    return balloon
