class Item:
    def __init__(self, label: str, value: float) -> None:
        self.label = label
        self.value = value


class Transaction:
    def __init__(self, id: float, twu: float, items: list[Item]) -> None:
        self.id = id
        self.items = items
        self.twu = twu


class RevisedDB:
    def __init__(self, dataset_path: str, min_util: float, min_corr: float) -> None:
        self.min_util = min_util
        self.min_corr = min_corr

        self.support: dict[str, float] = {}
        self.item_list: list[str] = []

        self.transactions: list[Transaction] = []
        self.readDB(dataset_path)

        print()
        print("Dataset Processing Completed")
        print("Dataset:", dataset_path)
        print("min_util:", self.min_util, ", min_corr:", self.min_corr)
        print("Items:", len(self.item_list))
        print("Transactions:", len(self.transactions))
        print("Total Combinations:", pow(2, len(self.item_list)) - 1)
        input("Enter to Continue...")

    def readDB(self, path: str):
        line_index = 0
        labels_twu = {}

        with open(path, "r") as f:
            for line in f.readlines():
                [labels, twu, utils] = line.split(":")
                twu = int(twu)

                label_list = labels.split(" ")
                utils_list = utils.split(" ")

                items: list[Item] = []
                for i in range(len(label_list)):
                    label = label_list[i].strip()
                    util = int(utils_list[i].strip())
                    self.support[label] = self.support.get(label, 0) + 1
                    labels_twu[label] = labels_twu.get(label, 0) + twu
                    items.append(Item(label, int(util)))

                # print([l.label for l in items])
                self.transactions.append(Transaction(line_index + 1, twu, items))
                line_index += 1

        revised_labels: set[str] = set()
        for i in range(len(self.transactions)):
            revised_twu = 0
            revised_items: list[Item] = []

            for item in self.transactions[i].items:
                if labels_twu[item.label] >= self.min_util:
                    # print(item.label, item.value)
                    revised_twu += item.value
                    revised_items.append(item)
                    revised_labels.add(item.label)

            self.transactions[i].items = revised_items
            self.transactions[i].twu = revised_twu

            self.item_list = list(revised_labels)
            self.item_list.sort(reverse=True, key=lambda item: self.support[item])
