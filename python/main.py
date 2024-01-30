import inquirer
import time
from inquirer.questions import os
from ECoHUP import ECoHUP, getPatterns
import matplotlib.pyplot as plt
import tabulate


def main():
    answers = inquirer.prompt([
        inquirer.List(
            'csv_folder',
            message="Select folder for input",
            choices=os.listdir("./csv"),
        ),
    ])

    if answers == None:
        return

    csv_folder = answers["csv_folder"]
    transactions_file = "./csv/{}/transactions.txt".format(csv_folder)
    utils_file = "./csv/{}/utils.txt".format(csv_folder)

    # Minimum utility and Minimum Correlation
    min_util = float(input("Enter min util: "))
    min_corr = float(input("Enter min corr: "))

    # Getting the patterns
    result = getPatterns(transactions_file, utils_file, min_util, min_corr)

    result.sort()
    print("\nPatterns: ", len(result))
    print(tabulate.tabulate([[item] for item in result],
                            ["Items"], "rounded_outline"))
    print()

    # Waiting the program
    input("Enter to continue...")

    # Find the unique items
    unique_items = sorted(list(
        set([item for row in result for item in row.split(" ")])
    ))

    answers = inquirer.prompt([
        inquirer.Checkbox('items',
                          message="Items to Pair",
                          choices=unique_items,
                          ),
    ])

    if answers == None:
        return

    compulsoryItems: set[str] = set(answers["items"])
    filtered_items = []
    for itemSet in result:
        is_subset = compulsoryItems.issubset(set(itemSet.split(" ")))
        if is_subset:
            filtered_items.append([itemSet])

    print("Filtered Item Set: ")
    print(tabulate.tabulate(filtered_items, [
          "Filtered Items"], "rounded_outline"))


def test():
    transactions_file = "./csv/foodmart/transactions.txt"
    utils_file = "./csv/foodmart/utils.txt"
    min_util: float = 1
    min_corr: float = 0.2

    pow = 1
    for _ in range(10):
        start_time = time.time()

        result = getPatterns(transactions_file, utils_file,
                             min_util * pow, min_corr)
        end_time = time.time()
        duration = end_time - start_time

        print("Patterns: ", len(result))
        print("Duration: ", duration)
        print(transactions_file, utils_file, min_util * pow, min_corr)

        pow = pow * 10

        print("#############\n")


if __name__ == "__main__":
    test()
