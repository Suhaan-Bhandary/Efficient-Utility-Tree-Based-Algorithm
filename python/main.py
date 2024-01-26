import inquirer
from inquirer.questions import os
from ECoHUP import getPatterns
import time


def main():
    answers = inquirer.prompt([
        inquirer.List(
            'csv_folder',
            message="Select folder for input: ",
            choices=os.listdir("./csv"),
        )
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


if __name__ == "__main__":
    main()
