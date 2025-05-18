## MurmurHash

## コンテキストハッシュ
###  典型的なハッシュ問題
サーバーキャッシュにおける、キャッシュの再配置

サーバーがネットワークから抜けたとき、そのサーバーが持っていたキャッシュを他のサーバーに移動させる必要がある。
新しいサーバーがネットワークに参加したとき、他のサーバーが持っているキャッシュを新しいサーバーに移動させる必要がある。

古典的なハッシュテーブルでは、新しいハッシュ関数を使ってデータを再配置してテーブルのサイズを変更することができる。
しかし、データの再配置は非常に高コストである。この作業がたまにならキャッシュによるメリットが上回るが、頻繁に行われるとキャッシュのメリットが失われてしまう。

### コンテキストハッシュによる解決
コンテキストハッシュ法では、リソースとノードの両方を固定範囲 `R = [0, 2^k-1]` にハッシュすることを考える。
Rを円周上に配置し、一番上を0とし、そこから時計回りに1, 2, ... , 2^k-1とする。
この円をハッシュリングと呼ぶ。
Webページは、ハッシュリングを時計回りに進んでいき最初に出会うノードに割り当てられる。

- ノードの検索
```python
    def lookupNode(self, hashValue):
        if self.legalRange(hashValue):
            temp = self.head
            if temp is None:
                return None
            else:
                while(self.distance(temp.hashValue, hashValue) >
                        self.distance(temp.next.hashValue, hashValue)):
                    temp = temp.next
                if temp.hashValue == hashValue:
                    return temp
                return temp.next
```

- リソースの移動
```python
  def moveResources(self, dest, orig, deleteTrue):
        delete_list = []
        for i, j in orig.resources.items():
            if (self.distance(i, dest.hashValue) < self.distance(i, orig.hashValue) or deleteTrue): /# dest(移動先)の方がorig(移動元)よりも近い場合
                dest.resources[i] = j
                delete_list.append(i)
                print("\tMoving a resource " + str(i) + " from " +
                      str(orig.hashValue) + " to " + str(dest.hashValue))
        for i in delete_list:
            del orig.resources[i] # 移動元からリソースを削除
```

- ノードの追加
```python
   def addNode(self, hashValue):
        if self.legalRange(hashValue):
            newNode = Node(hashValue)

            if self.head is None: # ハッシュリングが空の場合
                newNode.next = newNode
                newNode.previous = newNode
                self.head = newNode
                print("Adding a head node " + str(newNode.hashValue) + "...")
            else:
                temp = self.lookupNode(hashValue) # 新しいノードの位置を決定
                newNode.next = temp
                newNode.previous = temp.previous
                newNode.previous.next = newNode
                newNode.next.previous = newNode
                print("Adding a node " + str(newNode.hashValue) +
                      ". Its prev is " + str(newNode.previous.hashValue) +
                      ", and its next is " + str(newNode.next.hashValue) + ".")

                self.moveResources(newNode, newNode.next, False)

                if hashValue < self.head.hashValue: # 先頭ノードを更新する場合
                    self.head = newNode
```

-- ノードの削除
```python
    def removeNode(self, hashValue):
        temp = self.lookupNode(hashValue)
        if temp.hashValue == hashValue:
            print("Removing the node " + str(hashValue) + ": ")
            self.moveResources(temp.next, temp, True)
            temp.previous.next = temp.next
            temp.next.previous = temp.previous
            if self.head.hashValue == hashValue: # 先頭ノードを削除する場合
                self.head = temp.next
                if self.head == self.head.next: # ハッシュリングにノードが1つしかない場合 
                    self.head = None
            return temp.next
        else:
            print("Nothing to remove.")
```
