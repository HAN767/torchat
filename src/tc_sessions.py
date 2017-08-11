import json
import websockets
import asyncio

queue = list()
idDict = dict()

class AbstractSession:

    @staticmethod
    def is_openJSON_acceptable(jsonBuf):
        # may throw also exceptions from json module
        d = json.loads(jsonBuf)
        if type(d) is not dict or len(d) != 2 or 'open' not in d or 'from' not in d:
            raise ValueError("Not a valid JSON")
        return d

    def __init__(self, json, websocket):
        self.currentJSON = None
        self.type = json['open']
        self.websocket = websocket
        self._toClose = False
        self._errorState = ''
        self._acceptable = True
        self.nodeId = json['from']
        # store in idDict the new opened connection
        global idDict
        idDict[self.nodeId] = self

    def isAcceptable (self):
        return self._acceptable

    def toClose (self):
        return self._toClose

    def getErrorState(self):
        return self._errorState

    async def waitForAnotherJMU (self):
        buf = await self.websocket.recv()
        try:
            self.currentJSON = json.loads(buf)
            if type(self.currentJSON) is not dict:
                raise ValueError("json is not a dictionary")
            if 'close' in self.currentJSON:
                self._toClose = True
        except ValueError as e:
            self._acceptable = False
            self._toClose = True
            self._errorState = 'not valid JSON'

    def shutdown (self):
        raise NotImplementedError

    async def sendError(self):
        await self.websocket.send(self._errorState)


class MessageSession(AbstractSession):

    def __init__(self, json, websocket):
        super().__init__(json, websocket)
        if self.type != 'message':
            raise TypeError(self.type + ' is not supported as Message Session')

    async def waitForAnotherJMU(self):
        await super().waitForAnotherJMU()
        j = self.currentJSON
        if type(j) is not dict or 'msg' not in j or len(j) != 1:
            if 'close' not in j:
                # it is not a close jmu nor a message jmu
                # there was an error from the other node
                self._errorState = 'not a message json'
                self._acceptable = False
            return # do not store msg
        self.storeMessage()

    async def send(self, msg):
        d = {'msg': msg}
        j = json.dumps(d)
        await self.websocket.send(j)

    def storeMessage(self):
        global queue
        queue.append(self.formatMsg())

    def formatMsg(self):
        return (self.nodeId, self.currentJSON['msg'])

    def shutdown(self):
        self.websocket.close

class ClientSession(AbstractSession):

    def isAcceptable(self):
        # j = self.currentJSON
        # if j is not dict() or 'msg' not in j or len(j) != 1:
            # self.errorState = 'not a message json'
            # return False
        # else:
            # # json is acceptable and contains only 'msg' field
            # self.currentJSON = j
            return True

    async def waitForAnotherJMU(self):
        await super().waitForAnotherJMU()
        self.executeAction()

    def executeAction(self):
        j = self.currentJSON
        cmd = j['cmd']
        if cmd == 'getmsg':
            global queue
            if len(queue) == 0: 
                #TODO: correct way to inform queue is empty 
                j = json.dumps(dict())
            else:
                sendBack = queue.pop(0)
                j = json.dumps({'id' : sendBack[0], 'msg' : sendBack[1]})
            self.websocket.send(j)

        elif cmd == 'send':
            # TODO: not only message sessions
            global idDict
            if j['to'] not in idDict:
                d = {'from': j['to'], 'type': 'msg'}
                session = MessageSession(json.dumps(d), websocket)

            session = idDict[j['to']]
            session.send(j['msg'])

    def getLastMessageFromDaemon(self):
        j = json.dumps({'cmd': 'getmsg'})
        return await self.websocket.recv()

    def formatMsg(self):
        return {self.nodeId: self.currentJSON['msg']}

    def shutdown(self):
        self.websocket.close
