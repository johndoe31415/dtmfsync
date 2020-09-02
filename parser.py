import argparse
parser = argparse.ArgumentParser(prog = "tsfind", description = "Find timestamps in audio or video data", add_help = False)
parser.add_argument("filename", metavar = "filename", type = str, help = "Audio or video data to find encoded scrambled timestamps in.")
