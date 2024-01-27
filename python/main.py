import inquirer
import time
from inquirer.questions import os
from ECoHUP import ECoHUP, getPatterns
import matplotlib.pyplot as plt


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
    print(result)
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
    for itemSet in result:
        is_subset = compulsoryItems.issubset(set(itemSet.split(" ")))
        if is_subset:
            print(itemSet)


def test():
    transactions_file = "./csv/foodmart/transactions.txt"
    utils_file = "./csv/foodmart/utils.txt"
    min_util = 0.00010
    min_corr = 0.2

    start_time = time.time()
    ECoHUP(transactions_file, utils_file, min_util, min_corr)
    end_time = time.time()

    duration = end_time - start_time
    print(duration)


if __name__ == "__main__":
    main()
