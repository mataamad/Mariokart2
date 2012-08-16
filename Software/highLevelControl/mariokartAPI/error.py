'''
Mariokart project

Copyright 2012 University of Canterbury

Author: Matthew Wigley

This module deals with the board error state.

It provides functions to set the boards into an error state and to determine if the boards are in an error state.
'''

import UsbController
import definitions

#exception thrown when the boards are in their Error state and a request is made
class BoardError(Exception):
    pass

#sets the boards into their error state
def EnterErrorState():
    return UsbController.EnterErrorState()

#returns true if the boards are in the error state, false otherwise
def AreBoardsInErrorState():
    try:
        UsbController.GetValue(definitions.VAR_BRK_POS)
    except BoardError:
        return True

    return False
