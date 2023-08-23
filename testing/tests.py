import os
import sys
import time
import signal
import string
import random
import pynput
import pyperclip
import pyautogui
import subprocess

KEY_INTERVAL = 0.015
pkb = pynput.keyboard.Controller()

loremIpsum = """Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna
aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident,
sunt in culpa qui officia deserunt mollit anim id est laborum."""
loremIpsumLines = loremIpsum.split('\n')

def waitAndClickOnImage(imagePath, clicks=1):
	point = None
	while point is None:
		point = pyautogui.locateOnScreen(imagePath, grayscale=True, confidence=.8)
	pyautogui.moveTo(point)
	pyautogui.click(clicks=clicks)

def generateRandomText():
	digits = ''.join(random.sample(string.digits, 8))
	chars = ''.join(random.sample(string.ascii_letters, 15))
	return digits + chars

def getKey(key):
	if len(key) == 1:
		return f'"{key}"'
	return f"pynput.keyboard.Key.{key}"
def keyPress(key):
	time.sleep(KEY_INTERVAL)
	exec(f"pkb.press({getKey(key)})", globals(), locals())
	exec(f"pkb.release({getKey(key)})", globals(), locals())
def keyDown(key):
	time.sleep(KEY_INTERVAL)
	exec(f"pkb.press({getKey(key)})", globals(), locals())
def keyUp(key):
	time.sleep(KEY_INTERVAL)
	exec(f"pkb.release({getKey(key)})", globals(), locals())
def keyCombo(keys):
	for key in keys:
		time.sleep(KEY_INTERVAL)
		exec(f"pkb.press({getKey(key)})", globals(), locals())
	for key in reversed(keys):
		time.sleep(KEY_INTERVAL)
		exec(f"pkb.release({getKey(key)})", globals(), locals())
def keyWrite(text):
	pyautogui.write(text)
def setClipText(text):
	pyperclip.copy(text)
def getClipText():
	return pyperclip.paste()

logFile = open("stdout.txt", "w")
os.chdir("..")
if "nolaunch" in sys.argv:
	waitAndClickOnImage("testing/find_window.png")
else:
	p = subprocess.Popen(["bin/Release-windows-x86_64/scex/scex.exe"], stdout=logFile)

try:
	time.sleep(0.5)
	keyCombo(["ctrl", "n"])

	randomText = generateRandomText()
	setClipText(randomText)
	keyCombo(["ctrl", "v"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == randomText)
	print("Write and copy test passed")

	setClipText(loremIpsum)
	keyCombo(["ctrl", "v"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == loremIpsum)
	print("Paste from outside test passed")

	keyPress("left")
	keyWrite('\n')
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == '\n' + loremIpsum)
	print("Add line to beginning test passed")

	keyCombo(["ctrl", "a"])
	keyPress("right")
	keyWrite('\n')
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == '\n' + loremIpsum + '\n')
	print("Add line to end test passed")

	keyPress("right")
	keyCombo(["ctrl", "shift", "k"])
	keyPress("up")
	keyPress("up")
	keyCombo(["ctrl", "shift", "k"])
	keyCombo(["ctrl", "a"])
	keyPress("left")
	keyCombo(["ctrl", "shift", "k"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	clipboardLines = getClipText().split('\n')
	assert(len(clipboardLines) == 3)
	assert(clipboardLines[0] == loremIpsumLines[0])
	assert(clipboardLines[1] == loremIpsumLines[2])
	assert(clipboardLines[2] == loremIpsumLines[3])
	print("Ctrl shift k test passed")

	setClipText("asdf\n\tasdf\n\t\tasdf\n\nasdf\n\t\t\tasdf")
	keyCombo(["ctrl", "v"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "["])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "asdf\nasdf\n\tasdf\n\nasdf\n\t\tasdf")
	keyCombo(["ctrl", "["])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "asdf\nasdf\nasdf\n\nasdf\n\tasdf")
	keyCombo(["ctrl", "["])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "asdf\nasdf\nasdf\n\nasdf\nasdf")
	keyCombo(["ctrl", "]"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "\tasdf\n\tasdf\n\tasdf\n\n\tasdf\n\tasdf")
	print("Ctrl [ and ctrl ] test passed")

	setClipText("lalulalila babubibabuba")
	keyCombo(["ctrl", "v"])
	waitAndClickOnImage("testing/lalulalila.png", clicks=2)
	keyDown('ctrl')
	waitAndClickOnImage("testing/babubibabuba.png", clicks=2)
	keyUp('ctrl')
	keyWrite("zxcv")
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "zxcv zxcv")
	keyCombo(["ctrl", "z"])
	keyCombo(["ctrl", "z"])
	keyCombo(["ctrl", "z"])
	keyCombo(["ctrl", "z"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "lalulalila babubibabuba")
	keyCombo(["ctrl", "shift", "z"])
	keyCombo(["ctrl", "shift", "z"])
	keyCombo(["ctrl", "shift", "z"])
	keyCombo(["ctrl", "shift", "z"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "zxcv zxcv")
	print("Multicursor replace and undo redo test passed")

	setClipText("a\nbabubibabuba\nlalulalila")
	keyCombo(["ctrl", "v"])
	waitAndClickOnImage("testing/babubibabuba.png", clicks=2)
	keyPress("right")
	keyDown('ctrl')
	waitAndClickOnImage("testing/lalulalila.png", clicks=1)
	keyUp('ctrl')
	keyPress("delete")
	keyPress("delete")
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "a\nbabubibabubaalulila")
	print("Multicursor delete line test passed")

	setClipText("a\nbabubibabuba\nlalulalila")
	keyCombo(["ctrl", "v"])
	waitAndClickOnImage("testing/babubibabuba.png", clicks=2)
	keyPress("right")
	keyPress("right")
	keyDown('ctrl')
	waitAndClickOnImage("testing/lalulalila.png", clicks=1)
	keyUp('ctrl')
	keyPress("backspace")
	keyPress("backspace")
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "a\nbabubibabublalalila")
	print("Multicursor backspace line test passed")

	setClipText("babubibabuba lalulalila\nbabubibabuba lalulalila\nbabubibabuba lalulalila")
	keyCombo(["ctrl", "v"])
	waitAndClickOnImage("testing/babubibabuba.png", clicks=2)
	keyCombo(["ctrl", "d"])
	keyCombo(["ctrl", "d"])
	keyCombo(["ctrl", "c"])
	waitAndClickOnImage("testing/lalulalila.png", clicks=2)
	keyCombo(["ctrl", "d"])
	keyCombo(["ctrl", "d"])
	keyCombo(["ctrl", "v"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "babubibabuba babubibabuba\nbabubibabuba babubibabuba\nbabubibabuba babubibabuba")
	print("Ctrl d and multicursor copy paste test passed")

	waitAndClickOnImage("testing/babubibabuba.png", clicks=2)
	keyPress("right")
	keyCombo(["ctrl", "delete"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "babubibabubababubibabuba\nbabubibabuba babubibabuba\nbabubibabuba babubibabuba")
	waitAndClickOnImage("testing/babubibabuba.png", clicks=2)
	keyPress("delete")
	keyPress("delete")
	keyCombo(["ctrl", "delete"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == " babubibabuba\nbabubibabuba babubibabuba")
	keyCombo(["ctrl", "home"])
	keyCombo(["ctrl", "right"])
	keyCombo(["ctrl", "right"])
	keyPress("right")
	keyCombo(["ctrl", "right"])
	keyCombo(["ctrl", "right"])
	keyPress("right")
	keyPress("right")
	keyPress("space")
	keyCombo(["ctrl", "end"])
	keyCombo(["ctrl", "left"])
	keyCombo(["ctrl", "left"])
	keyCombo(["ctrl", "left"])
	keyPress("delete")
	keyCombo(["ctrl", "left"])
	keyCombo(["ctrl", "backspace"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == " babubibabuba\n a bubibabuba")
	print("Word mode move and delete test passed")

	setClipText("asdf fdsa\nasdf fdsa\nbabubibabuba lalulalila\nasdf fdsa\nasdf fdsa")
	keyCombo(["ctrl", "v"])
	waitAndClickOnImage("testing/babubibabuba.png", clicks=1)
	keyCombo(["ctrl", "shift", "k"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "asdf fdsa\nasdf fdsa\nasdf fdsa\nasdf fdsa")
	keyCombo(["ctrl", "z"])
	waitAndClickOnImage("testing/babubibabuba.png", clicks=1)
	keyDown('ctrl')
	waitAndClickOnImage("testing/lalulalila.png", clicks=1)
	keyUp('ctrl')
	keyCombo(["ctrl", "shift", "k"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "asdf fdsa\nasdf fdsa\nasdf fdsa\nasdf fdsa")
	print("Multicursor ctrl shift k test passed")

	setClipText("alsdkj flkaj sdlf\ndfssfd lalulalila\nalksd flkajs dlfk\nbabubibabuba\nalksd flkasjd fal")
	keyCombo(["ctrl", "v"])
	waitAndClickOnImage("testing/babubibabuba.png", clicks=1)
	keyDown('shift')
	waitAndClickOnImage("testing/lalulalila.png", clicks=1)
	keyUp('shift')
	keyCombo(["ctrl", "shift", "k"])
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "alsdkj flkaj sdlf\nalksd flkasjd fal")
	print("Ctrl shift k with multiline selection test passed")

	setClipText(" \t\n  \t\n   \t\n    \t")
	keyCombo(["ctrl", "v"])
	keyCombo(["ctrl", "end"])
	keyCombo(["shift", "left"])
	keyCombo(["ctrl", "d"])
	keyCombo(["ctrl", "d"])
	keyCombo(["ctrl", "d"])
	keyPress("delete")
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == " \n  \n   \n    ")
	print("Tricky ctrl d test A passed")
	keyCombo(["ctrl", "z"])
	keyCombo(["ctrl", "a"])
	keyCombo(["left"])
	keyCombo(["shift", "right"])
	keyCombo(["shift", "right"])
	keyCombo(["ctrl", "d"])
	keyCombo(["ctrl", "d"])
	keyCombo(["ctrl", "d"])
	keyPress("delete")
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "\n \n  \n   ")
	print("Tricky ctrl d test B passed")
	keyCombo(["ctrl", "z"])
	keyCombo(["ctrl", "a"])
	keyCombo(["left"])
	keyCombo(["shift", "right"])
	keyCombo(["shift", "right"])
	keyCombo(["shift", "right"])
	keyCombo(["ctrl", "d"])
	keyCombo(["ctrl", "d"])
	keyCombo(["ctrl", "d"])
	keyCombo(["ctrl", "d"]) # one extra time
	keyPress("delete")
	keyCombo(["ctrl", "a"])
	keyCombo(["ctrl", "c"])
	assert(getClipText() == "       \t")
	print("Multiline ctrl d test passed")
	
# 	breakpoint()

except Exception as e:
	os.kill(p.pid, signal.SIGTERM)
	raise e

print("All tests passed")
os.kill(p.pid, signal.SIGTERM)
logFile.close()