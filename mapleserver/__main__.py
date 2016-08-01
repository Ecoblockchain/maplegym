import time
import master


A_LONG_TIME = 999999


if __name__ == '__main__':
    server = master.start()
    try:
        time.sleep(A_LONG_TIME)
    except KeyboardInterrupt:
        print '[maplegym] CTRL + C detected, exiting...'
        server.stop()
