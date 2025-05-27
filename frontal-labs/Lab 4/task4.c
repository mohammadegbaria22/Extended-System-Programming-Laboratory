    int digit_counter(char *number)
    {
        int counter = 0;
        for (int i = 0; number[i] != '\0'; i++)
        {
            if (number[i] <= '9' && number[i] >= '0')
                counter++;
        }
        return counter;
    }

    int main(int argc, char **argv)
    {
        if (argc < 2)
        {
            exit(0);
        }
    }


