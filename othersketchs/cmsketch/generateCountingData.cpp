void generateCountingData(const char *inName, const char *outName, uint max_counter, uint insert_num) {

	FILE *fin, *fout;

	fopen_s(&fin, inName, "r");
	fopen_s(&fout, outName, "w");

	srand(time(NULL));
	int cnt = 0;

	bool end_file = false;

	while (true) {
		int x;
		for (int i = 0; i < 13; i++) {
			if (fscanf_s(fin, "%d", &x) == EOF) {
				end_file = true;
				break;
			}
			fprintf(fout, "%d ", x);
		}
		if (end_file) break;
		if (cnt < insert_num) fprintf(fout, ": %d", rand() % max_counter + 1);
		fprintf(fout, "\n");
		cnt++;
	}
	fclose(fin);
	fclose(fout);

}