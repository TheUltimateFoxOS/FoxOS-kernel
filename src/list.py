import glob
import os
import shutil
import re
import json
import sys

FOXOS_DIR = "./FoxOS-kernel/src"
DOCS_DIR = "./FoxOS-kernel/doc"
DOC_TMP_BUILD_FILE = "/tmp/"
num_functions = 0
num_functions_desc = 0

file_template_html = """<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
    <link rel="stylesheet" href="/style.css">
    <link rel="stylesheet" href="/w3.css">
    <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Noto+Sans+JP&display=swap">
    <title>FoxOS - Docs</title>
  </head>
  <body>
    <div class="w3-top">
      <div class="w3-bar w3-border w3-border-orange w3-orange w3-card w3-large fox-bar">
        <a href="/" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">Home</a>
        <a href="/about/" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">About us</a>
        <a href="/FoxOS-kernel/" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">Docs</a>
        <a href="https://github.com/TheUltimateFoxOS" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">GitHub</a>
      </div>
    </div>
    
    <header class="w3-container w3-orange w3-center" style="padding:50px 16px">
      <h1 class="w3-margin w3-jumbo">FoxOS</h1>
      <p class="w3-xlarge">FoxOS docs - {%FILENAME%}</p>
    </header>
    
    <div class="fox-text">
"""

index_file_template = """<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
    <link rel="stylesheet" href="/style.css">
    <link rel="stylesheet" href="/w3.css">
    <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Noto+Sans+JP&display=swap">
    <title>FoxOS - Docs</title>
  </head>
  <body>
    <div class="w3-top">
      <div class="w3-bar w3-border w3-border-orange w3-orange w3-card w3-large fox-bar">
        <a href="/" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">Home</a>
        <a href="/about/" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">About us</a>
        <a href="/FoxOS-kernel/" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">Docs</a>
        <a href="https://github.com/TheUltimateFoxOS" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">GitHub</a>
      </div>
    </div>
    
    <header class="w3-container w3-orange w3-center" style="padding:50px 16px">
      <h1 class="w3-margin w3-jumbo">FoxOS</h1>
      <p class="w3-xlarge">FoxOS docs</p>
    </header>
    
    <div class="fox-text">
      <ul class="w3-ul w3-hoverable fox-a-nodecoration fox-ul">
"""

no_content_template = """<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
    <link rel="stylesheet" href="/style.css">
    <link rel="stylesheet" href="/w3.css">
    <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Noto+Sans+JP&display=swap">
    <title>FoxOS - Docs</title>
  </head>
  <body>
    <div class="w3-top">
      <div class="w3-bar w3-border w3-border-orange w3-orange w3-card w3-large fox-bar">
        <a href="/" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">Home</a>
        <a href="/about/" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">About us</a>
        <a href="/FoxOS-kernel/" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">Docs</a>
        <a href="https://github.com/TheUltimateFoxOS" class="w3-bar-item w3-button w3-hover-none w3-border-orange w3-bottombar w3-hover-border-black">GitHub</a>
      </div>
    </div>
    
    <header class="w3-container w3-orange w3-center" style="padding:50px 16px">
      <h1 class="w3-margin w3-jumbo">FoxOS</h1>
      <p class="w3-xlarge">FoxOS docs - {%FILENAME%}</p>
    </header>
    
    <div class="fox-text">
      <p>No content to display.</p>
      <p>If you are seeing this page, it means that the documentation you are searching does not exist! For help with this file please <a href="https://theultimatefoxos.github.io/">contact us!</a></p>
    </div>
  </body>
</html>
"""

def loadtxt(filename):
	with open(filename, "r") as f:
		return f.read()

def get_file_functions(file, write_to, name):
	entries_found = 0

	newfile = open(write_to, "w")
	newfile.write(file_template_html.replace("{%FILENAME%}", name))
	newfile.close()

	global num_functions_desc
	global num_functions

	if (file.endswith(".asm")):
		#If an entry is found, run this: entries_found += 1
		rproc = r"^([\w.]+: *)(?![\w ]+)"
		code = loadtxt(file)
		procs = re.finditer(rproc, code, flags=re.MULTILINE)
		for idx, i in enumerate(procs):

			if i.group().strip()[:-1][0] == ".":
				continue

			sig_find_regex = r"^; ?# ?" + i.group().strip()[:-1] + r"-signature: ?([\w\d ;\":_\-#+\*.,'()]*);?"
			sig = re.finditer(sig_find_regex, code, flags=re.MULTILINE)
			sig_str = ""
			for x in sig:
				sig_str = x.group(1).strip()
				if sig_str[-1] == ";":
					sig_str = sig_str[:-1]
				if not sig_str.startswith("extern \"C\""):
					sig_str = "extern \"C\" " + sig_str
			#print(sig_str)

			doc_find_regex = r"^; ?# ?" + i.group().strip()[:-1] + r"-doc: ?([\w\d ;:_\-#+\*.,'()]*);?"
			doc = re.finditer(doc_find_regex, code, flags=re.MULTILINE)
			doc_str = ""

			for x in doc:
				doc_str += x.group(1)
			
			maybe_discard = r"; ?# ?" + i.group().strip()[:-1] + r"-discard"
			if (re.search(maybe_discard, code)):
				continue
			

			if sig_str == "":
				print("\033[33m" + "No signature found for: " + i.group().strip()[:-1] + "\033[0m")
				continue
			else:
				entries_found += 1
			
			out = sig_str.strip()

			return_type = out.split("(", 1)[0].strip()
			attrubutes = ""
			if (len(return_type.split(" ")) == 1):
				function_name = return_type
				return_type = ""
			else:
				ret_array = return_type.split(" ")
				ret_len = len(ret_array) - 1
				function_name = ret_array[ret_len].strip()

				ret_array.pop()
				ret_len = len(ret_array)
				
				return_type = ret_array[ret_len-1]
				is_ret = True
				for i in reversed(range(ret_len-1)):
					if ((ret_array[i] == "short" or ret_array[i] == "long" or ret_array[i] == "signed" or ret_array[i] == "unsigned" or ret_array[i] == "const" or ret_array[i] == "restrict" or ret_array[i] == "auto" or ret_array[i] == "inline" or ret_array[i] == "mutable") and is_ret):
						return_type = ret_array[i] + " " + return_type
					else:
						is_ret = False
						attrubutes = ret_array[i] + " " + attrubutes

				attrubutes = attrubutes.strip()
				return_type = return_type.strip()

			class_name = ""
			if (len(function_name.split("::", 1)) != 1):
				class_name = function_name.split("::", 1)[0]
				function_name = function_name.split("::", 1)[1]

			if (function_name.startswith("*")):
				function_name = function_name[1:]
				return_type = return_type + "*"

			params = out.split("(", 1)[1].split(")", 1)[0]

			#print(attrubutes + " --- " + return_type + " --- " + class_name + " --- " + function_name + " --- " + params)

			num_functions += 1

			doc_name = ""
			if (class_name != ""):
				doc_name = class_name + "::" + function_name
			else:
				doc_name = function_name
			
			doc_find_regex = r"^; ?# ?" + doc_name + r"-doc: ?([\w\d ;:_\-#+\*.,'()]*);?"
			doc = re.finditer(doc_find_regex, code, flags=re.MULTILINE)
			doc_str = ""

			for x in doc:
				doc_str += x.group(1)

			text = "      <div>\n"
			text = text + "        <h2>" + function_name + "</h2>\n"
			text = text + "        <b>Signature:</b> <code>" + out + "</code><br/>\n"
			if (class_name != ""):
				text = text + "        <b>Class/Namespace:</b> <code>" + class_name + "</code><br/>\n"
			if (attrubutes != ""):
				text = text + "        <b>Attributes:</b> <code>" + attrubutes + "</code><br/>\n"
			if (doc_str != ""):
				num_functions_desc += 1

				text = text + f"        <b>Description:</b> {doc_str}<br/>\n"
			else:
				print("\x1b[31mNo description found for " + doc_name + " in " + file + "\x1b[0m")
			if (params != ""):
				text = text + "        <br/>\n"
				text = text + "        <b>Parameters:</b><br/>\n"
				text = text + "        <ul>\n"
				for param_for in params.split(","):
					param = param_for.strip()
					text = text + "          <li><code>" + param + "</code></li>\n"
				text = text + "        </ul>\n"
			if (return_type != ""):
				text = text + "        <br/>\n"
				text = text + "        <b>Return:</b> <code>" + return_type + "</code>\n"
			text = text + "      </div>\n"
			text = text + "      <hr>\n"

			f = open(write_to, "a")
			f.write(text)
			f.close()

		#with open(DEST, "w") as f:
		#	f.write(no_content_template.replace("{%FILENAME%}", file.split("/")[-1]))
	elif (file.endswith(".cpp")):
		rproc = r"(([\w \*:]+)(?!.*(=)).+)((?<=[\s:~])((?!.*(if|switch$|do$|for$|while|\[$|\]$)).+)\s*\(([\w\s,<>\[\].=&':/*+]*?)\)\s*(const)?\s*(?={))"
		code = loadtxt(file)
		procs = re.finditer(rproc, code)

		for idx, i in enumerate(procs):
			entries_found += 1
			out = i.group().replace("	", "").strip()

			return_type = out.split("(", 1)[0].strip()
			attrubutes = ""
			if (len(return_type.split(" ")) == 1):
				function_name = return_type
				return_type = ""
			else:
				ret_array = return_type.split(" ")
				ret_len = len(ret_array) - 1
				function_name = ret_array[ret_len].strip()

				ret_array.pop()
				ret_len = len(ret_array)
				
				return_type = ret_array[ret_len-1]
				is_ret = True
				for i in reversed(range(ret_len-1)):
					if ((ret_array[i] == "short" or ret_array[i] == "long" or ret_array[i] == "signed" or ret_array[i] == "unsigned" or ret_array[i] == "const" or ret_array[i] == "restrict" or ret_array[i] == "auto" or ret_array[i] == "inline" or ret_array[i] == "mutable") and is_ret):
						return_type = ret_array[i] + " " + return_type
					else:
						is_ret = False
						attrubutes = ret_array[i] + " " + attrubutes

				attrubutes = attrubutes.strip()
				return_type = return_type.strip()

			class_name = ""
			if (len(function_name.split("::", 1)) != 1):
				class_name = function_name.split("::", 1)[0]
				function_name = function_name.split("::", 1)[1]

			if (function_name.startswith("*")):
				function_name = function_name[1:]
				return_type = return_type + "*"

			params = out.split("(", 1)[1].split(")", 1)[0]

			#print(attrubutes + " --- " + return_type + " --- " + class_name + " --- " + function_name + " --- " + params)
		
			doc_name = ""
			if (class_name != ""):
				doc_name = class_name + "::" + function_name
			else:
				doc_name = function_name
			doc_name = doc_name.replace("[", "\\[")
			doc_name = doc_name.replace("]", "\\]")

			maybe_discard = r"\/{2} ?# ?" + doc_name + r"-discard"
			if (re.search(maybe_discard, code)):
				continue

			num_functions += 1

			doc_find_regex = r"\/{2} ?# ?" + doc_name + r"-doc: ?([\w\d ;:_\-#+\*.,']*)"
			doc = re.finditer(doc_find_regex, code)
			doc_str = ""

			for x in doc:
				doc_str += x.group(1)

			text = "      <div>\n"
			text = text + "        <h2>" + function_name + "</h2>\n"
			text = text + "        <b>Signature:</b> <code>" + out + "</code><br/>\n"
			if (class_name != ""):
				text = text + "        <b>Class/Namespace:</b> <code>" + class_name + "</code><br/>\n"
			if (attrubutes != ""):
				text = text + "        <b>Attributes:</b> <code>" + attrubutes + "</code><br/>\n"
			if (doc_str != ""):
				num_functions_desc += 1
				text = text + f"        <b>Description:</b> {doc_str}<br/>\n"
			else:
				print("\x1b[31mNo description found for " + doc_name + " in " + file + "\x1b[0m")
			if (params != ""):
				text = text + "        <br/>\n"
				text = text + "        <b>Parameters:</b><br/>\n"
				text = text + "        <ul>\n"
				for param_for in params.split(","):
					param = param_for.strip()
					text = text + "          <li><code>" + param + "</code></li>\n"
				text = text + "        </ul>\n"
			if (return_type != ""):
				text = text + "        <br/>\n"
				text = text + "        <b>Return:</b> <code>" + return_type + "</code>\n"
			text = text + "      </div>\n"
			text = text + "      <hr>\n"

			f = open(write_to, "a")
			f.write(text)
			f.close()

	if (entries_found == 0):
		newfile = open(write_to, "a")
		newfile.write("      <b>No functions found.</b>")
		newfile.close()

	newfile = open(write_to, "a")
	newfile.write("    </div>\n  </body>\n</html>")
	newfile.close()

def _needs_skip(file):
	if (file in "/fs/fat32/diskio.cpp" or file in "/fs/fat32/ff.cpp" or file in "/fs/fat32/ffsystem.cpp" or file in "/fs/fat32/ffunicode.cpp"):
		return True
	else:
		return False

def print_file(file, only_index, ptmp):
	if _needs_skip(file):
		return

	filetmp = file.replace(FOXOS_DIR, DOCS_DIR)
	if (filetmp.endswith("/")):
		filetmp = filetmp[:-1]

	head, tail = os.path.split(filetmp)

	if filetmp.endswith(".asm"):
		f = open(ptmp + "/index.html", "a")
		f.write("        <li><a href=\"" + filetmp.replace(DOCS_DIR, "/doc").split(".")[:-1][0] + "_asm.html\">" + tail + "</a></li>\n")
		f.close()
	elif filetmp.endswith(".cpp"):
		f = open(ptmp + "/index.html", "a")
		f.write("        <li><a href=\"" + filetmp.replace(DOCS_DIR, "/doc").split(".")[:-1][0] + ".html\">" + tail + "</a></li>\n")
		f.close()
	else:
		raise Exception("Unknown file type: " + file)

	if not os.path.exists(head):
		os.makedirs(head)

	if (only_index == False):
		get_file_functions(file, filetmp + ".html", tail)

def find_cpp(path, only_index, is_root):
	if (path.endswith("/")):
		path = path[:-1]

	ptmp = path.replace(FOXOS_DIR, DOCS_DIR)
	phead, ptail = os.path.split(ptmp)

	if not os.path.exists(ptmp):
		os.makedirs(ptmp)

	if (is_root == False):
		f = open(ptmp + "/index.html", "a")
		f.write("        <li><a href=\"" + phead.replace(DOCS_DIR, "/doc") + "\">..</a></li>\n")
		f.close()

	files = glob.glob(path + "/*")
	for tmp in files:
		filetmp = tmp.replace(FOXOS_DIR, DOCS_DIR)

		if (filetmp == DOCS_DIR + "/include"):
			pass
		elif (os.path.isdir(tmp)):
			if not os.path.exists(filetmp):
				os.makedirs(filetmp)

			webpath = filetmp.replace(DOCS_DIR, "/doc")
			webhead, webtail = os.path.split(webpath)
			head, tail = os.path.split(filetmp)

			f = open(ptmp + "/index.html", "a")
			f.write("        <li><a href=\"" + webpath + "/\">" + webtail + "/</a></li>\n")
			f.close()

			with open(filetmp + "/index.html", "w") as f:
				f.write(index_file_template)
			find_cpp(tmp, only_index, False)
			f = open(filetmp + "/index.html", "a")
			f.write("      </ul>\n    </div>\n  </body>\n</html>")
			f.close()
		else:
			if (tmp.endswith(".cpp") or tmp.endswith(".asm")):
				print_file(tmp, only_index, ptmp)

def print_progress(progress):
	toolbar_width = 80

	# setup toolbar
	empty_bar = " " * toolbar_width
	sys.stdout.write(f"{round(progress)}% [{empty_bar}]")
	sys.stdout.flush()
	sys.stdout.write("\b" * (toolbar_width + 1)) # return to start of line, after '['

	for i in range(int((toolbar_width - 1) / 100 * progress)):
		sys.stdout.write("-")
		sys.stdout.flush()

	sys.stdout.write(">\n") # this ends the progress bar

if sys.argv[1] == "index":
	FOXOS_DIR = sys.argv[2]
	DOCS_DIR = sys.argv[3]

	if not os.path.exists(DOCS_DIR):
		os.makedirs(DOCS_DIR)
	
	#print(DOCS_DIR + "/index.html")
	with open(DOCS_DIR + "/index.html", "w") as f:
		f.write(index_file_template)

	print(FOXOS_DIR)
	find_cpp(FOXOS_DIR, True, True)
	with open(DOCS_DIR + "/index.html", "a") as f:
		f.write("      </ul>\n    </div>\n  </body>\n</html>")
elif sys.argv[1] == "sumary":
	DOC_TMP_BUILD_FILE += sys.argv[2]
	with open(DOC_TMP_BUILD_FILE, "r") as f:
		data = json.load(f)
		#print(data)
		
		num_functions = data["_num_functions"]
		num_functions_desc = data["_num_functions_desc"]
	print("Documented " + str(num_functions) + " functions from witch " + str(num_functions_desc) + " have a description. This are " + str(round(num_functions_desc / num_functions * 100)) + "% of the functions.")

	print_progress((num_functions_desc / num_functions) * 100)
else:
	SOURCE = sys.argv[1]
	DEST = sys.argv[2]
	DOC_TMP_BUILD_FILE += sys.argv[3]
	
	if not os.path.isfile(DOC_TMP_BUILD_FILE):
		with open(DOC_TMP_BUILD_FILE, "w") as f:
			f.write(json.dumps({
				"_num_functions": 0,
				"_num_functions_desc": 0
			}))
	else:
		with open(DOC_TMP_BUILD_FILE, "r") as f:
			data = json.load(f)
			#print(data)
			
			num_functions = data["_num_functions"]
			num_functions_desc = data["_num_functions_desc"]

	#print(SOURCE, DEST)
	#print(num_functions, num_functions_desc)
	if _needs_skip(SOURCE):
		with open(DEST, "w") as f:
			f.write(no_content_template.replace("{%FILENAME%}", SOURCE.split("/")[-1]))
	else:
		get_file_functions(SOURCE, DEST, SOURCE.split("/")[-1])
	
	with open(DOC_TMP_BUILD_FILE, "w") as f:
		f.write(json.dumps({
			"_num_functions": num_functions,
			"_num_functions_desc": num_functions_desc
		}))