# Import pickle module
import pickle

# Declare the object to store data
dataObject = []
# Iterate the for loop for 5 times
for num in range(10,15):
   dataObject.append(num)

# Open a file for writing data
file_handler = open('languages', 'wb')
# Dump the data of the object into the file
pickle.dump(dataObject, file_handler)
# close the file handler
file_handler.close()

# Open a file for reading the file
file_handler = open('languages', 'rb')
# Load the data from the file after deserialization
dataObject = pickle.load(file_handler)
# Iterate the loop to print the data
for val in dataObject:
  print('The data value : ', val)
# close the file handler
file_handler.close()

# Define the class
class Employee:
    name = "Mostak Mahmud"
    # Define the method
    def details(self):
        print("Post: Marketing Officer")
        print("Department: Sales")
        print("Salary: $1000")

# Create the employee object    
emp = Employee()
# Print the class variable
print("Name:",emp.name)
# Call the class method
emp.details()
