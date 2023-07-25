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
import pydirectinput

HOTKEY_INTERVAL = 0.06
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


os.chdir("..")
if "nolaunch" in sys.argv:
	waitAndClickOnImage("testing/find_window.png")
else:
	p = subprocess.Popen(["bin/Release-windows-x86_64/ste/ste.exe"], stdout=subprocess.DEVNULL)

try:
	time.sleep(0.5)
	pyautogui.hotkey("ctrl", "n", interval=HOTKEY_INTERVAL)

	randomText = generateRandomText()
	pyperclip.copy(randomText)
	pyautogui.hotkey("ctrl", "v", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == randomText)
	print("Write and copy test passed")

	pyperclip.copy(loremIpsum)
	pyautogui.hotkey("ctrl", "v", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == loremIpsum)
	print("Paste from outside test passed")

	pydirectinput.press("left")
	pyautogui.write('\n')
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == '\n' + loremIpsum)
	print("Add line to beginning test passed")

	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pydirectinput.press("right")
	pyautogui.write('\n')
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == '\n' + loremIpsum + '\n')
	print("Add line to end test passed")

	pydirectinput.press("right")
	pyautogui.hotkey("ctrl", "shift", "k", interval=HOTKEY_INTERVAL)
	pydirectinput.press("up")
	pydirectinput.press("up")
	pyautogui.hotkey("ctrl", "shift", "k", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pydirectinput.press("left")
	pyautogui.hotkey("ctrl", "shift", "k", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	clipboardLines = pyperclip.paste().split('\n')
	assert(len(clipboardLines) == 3)
	assert(clipboardLines[0] == loremIpsumLines[0])
	assert(clipboardLines[1] == loremIpsumLines[2])
	assert(clipboardLines[2] == loremIpsumLines[3])
	print("Ctrl shift k test passed")

	pyperclip.copy("asdf\n\tasdf\n\t\tasdf\n\nasdf\n\t\t\tasdf")
	pyautogui.hotkey("ctrl", "v", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "[", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == "asdf\nasdf\n\tasdf\n\nasdf\n\t\tasdf")
	pyautogui.hotkey("ctrl", "[", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == "asdf\nasdf\nasdf\n\nasdf\n\tasdf")
	pyautogui.hotkey("ctrl", "[", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == "asdf\nasdf\nasdf\n\nasdf\nasdf")
	pyautogui.hotkey("ctrl", "]", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == "\tasdf\n\tasdf\n\tasdf\n\n\tasdf\n\tasdf")
	print("Ctrl [ and ctrl ] test passed")

	pyperclip.copy("lalulalila babubibabuba")
	pyautogui.hotkey("ctrl", "v", interval=HOTKEY_INTERVAL)
	waitAndClickOnImage("testing/lalulalila.png", clicks=2)
	pyautogui.keyDown('ctrl')
	waitAndClickOnImage("testing/babubibabuba.png", clicks=2)
	pyautogui.keyUp('ctrl')
	pyautogui.write("zxcv")
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == "zxcv zxcv")
	pyautogui.hotkey("ctrl", "z", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "z", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "z", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "z", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == "lalulalila babubibabuba")
	pyautogui.hotkey("ctrl", "shift", "z", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "shift", "z", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "shift", "z", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "shift", "z", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == "zxcv zxcv")
	print("Multicursor replace and undo redo test passed")

	pyperclip.copy("a\nbabubibabuba\nlalulalila")
	pyautogui.hotkey("ctrl", "v", interval=HOTKEY_INTERVAL)
	waitAndClickOnImage("testing/babubibabuba.png", clicks=2)
	pydirectinput.press("right")
	pyautogui.keyDown('ctrl')
	waitAndClickOnImage("testing/lalulalila.png", clicks=1)
	pyautogui.keyUp('ctrl')
	pkb.press(pynput.keyboard.Key.delete)
	pkb.release(pynput.keyboard.Key.delete)
	pkb.press(pynput.keyboard.Key.delete)
	pkb.release(pynput.keyboard.Key.delete)
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == "a\nbabubibabubaalulila")
	print("Multicursor delete line test passed")

	pyperclip.copy("a\nbabubibabuba\nlalulalila")
	pyautogui.hotkey("ctrl", "v", interval=HOTKEY_INTERVAL)
	waitAndClickOnImage("testing/babubibabuba.png", clicks=2)
	pydirectinput.press("right")
	pydirectinput.press("right")
	pyautogui.keyDown('ctrl')
	waitAndClickOnImage("testing/lalulalila.png", clicks=1)
	pyautogui.keyUp('ctrl')
	pyautogui.press('backspace')
	pyautogui.press('backspace')
	pyautogui.hotkey("ctrl", "a", interval=HOTKEY_INTERVAL)
	pyautogui.hotkey("ctrl", "c", interval=HOTKEY_INTERVAL)
	assert(pyperclip.paste() == "a\nbabubibabublalalila")
	print("Multicursor backspace line test passed")

# 	breakpoint()

except Exception as e:
	os.kill(p.pid, signal.SIGTERM)
	raise e

print("All tests passed")
os.kill(p.pid, signal.SIGTERM)