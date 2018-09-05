#include "databasemanager.h"
DatabaseManager::DatabaseManager() {
    use="";
}

void DatabaseManager::create_database(string name, unsigned int database_size){
    if(name.size() > MAX_STRING_SIZE){
        cout << "Error: database name should not be larger than 20 characters" << endl;
        return;
    }
    if(database_size > MAXIMUN_DATABASE_SIZE_GB){
        cout<<"Database cannot exceed "<<MAXIMUN_DATABASE_SIZE_GB<<" GB"<<endl;
        return;
    }

    strcpy(dbh.sb.name,name.c_str());
    dbh.sb.database_size = from_MB_bytes_convertion(database_size, TO_BYTE);
    dbh.sb.blocks_count = dbh.sb.database_size / BLOCK_SIZE;
    dbh.sb.itables_count = (dbh.sb.database_size*PERCENTAGE_FOR_ITABLES) /ITABLE_SIZE;
    dbh.sb.free_itables_count = dbh.sb.itables_count;

    use = name;
    name += ".dat";
    string database_path =  PATH + name;
    ofstream output_file(database_path.c_str(),ios::binary);
    output_file.seekp(SB_SIZE, ios::beg);

    struct i_table it;
    memset(it.name,0,MAX_STRING_SIZE);
    it.first_block = -1;
    it.records_count = -1;
    it.fields_count = -1;
    it.table_size = -1;
    it.record_size = -1;
    it.index = -1;
    for(int i = 0; i < dbh.sb.itables_count; i++) {
        output_file.write(((char*)&it), ITABLE_SIZE);
    }

    dbh.sb.ptr_itable_bipmap = output_file.tellp();
    dbh.itable_bitmap_size = dbh.sb.itables_count/CHAR_BITS_SIZE;
    dbh.itable_bitmap = new char[dbh.itable_bitmap_size];
    memset((void*)dbh.itable_bitmap,0, dbh.itable_bitmap_size );
    output_file.write(dbh.itable_bitmap, dbh.itable_bitmap_size);

    dbh.sb.ptr_blocks_bitmap = output_file.tellp();
    dbh.blocks_bitmap_size = dbh.sb.blocks_count / CHAR_BITS_SIZE;
    dbh.blocks_bitmap = new char[dbh.blocks_bitmap_size];
    memset((void*)dbh.blocks_bitmap,0, dbh.blocks_bitmap_size);
    unsigned int position = output_file.tellp();
    output_file.seekp(position + dbh.blocks_bitmap_size, ios::beg);

    unsigned int FD_size = output_file.tellp();

    unsigned int fs_blocks_used = ceil (((double)FD_size / (double)BLOCK_SIZE));
    dbh.sb.free_blocks_count = dbh.sb.blocks_count-fs_blocks_used;
    
    for(int i=0; i < fs_blocks_used; i++) {
        setBlock_use(dbh.blocks_bitmap,i);
    }
    
    output_file.seekp(dbh.sb.ptr_blocks_bitmap,ios::beg);
    output_file.write(dbh.blocks_bitmap, dbh.blocks_bitmap_size);
    dbh.sb.FD_size = fs_blocks_used * BLOCK_SIZE;
    dbh.sb.free_database_space = dbh.sb.database_size - dbh.sb.FD_size;
    output_file.seekp(0, ios::beg);
    output_file.write(((char*)&dbh.sb), SB_SIZE);

    char block[BLOCK_SIZE];
    memset(block,0,BLOCK_SIZE);
    output_file.seekp(dbh.sb.database_size-BLOCK_SIZE, ios::beg);
    output_file.write(block, BLOCK_SIZE);
    output_file.close();
    cout << "\n\n\t\tDatabase " << use << " was created.\n\n" << endl;
}

void DatabaseManager::print_database_info(){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    cout << "\n\n\t\tDatabase Size: " << dbh.sb.database_size << endl;
    cout << "\t\tFree Space: " << dbh.sb.free_database_space << endl;
    //cout<<"Metadata Size: "<<dbh.sb.FD_size<<endl;
    //cout<<"Blocks Count: "<<dbh.sb.blocks_count<<endl;
    //cout<<"Free Blocks Count: "<<dbh.sb.free_blocks_count<<endl;
    //cout<<"Itables Count: "<<dbh.sb.itables_count<<endl;
    //cout<<"Free Itables Count: "<<dbh.sb.free_itables_count<<endl;
    //cout<<"Itable Bitmap Pointer: "<<dbh.sb.ptr_itable_bipmap<<endl;
    //cout<<"Blocks Bitmap Pointer: "<<dbh.sb.ptr_blocks_bitmap<<endl;
    //cout<<"Next Free Block: "<<next_available(dbh.blocks_bitmap,dbh.sb.blocks_count)<<endl;
    //cout<<"Next Free Itable: "<<next_available(dbh.itable_bitmap,dbh.sb.itables_count)<<endl;
}

void DatabaseManager::use_database(string name){
    use = name;
    name += ".dat";
    string database_path =  PATH + name;

    ifstream in(database_path.c_str(),ios::in | ios::out | ios::binary);

    if (!in) {
        cout << "Error while trying to open database: " << name << endl;
        use = "";
        return;
    }

    in.read(((char*) &dbh.sb), SB_SIZE);

    dbh.itable_bitmap_size = dbh.sb.itables_count / CHAR_BITS_SIZE;
    dbh.blocks_bitmap_size = dbh.sb.blocks_count / CHAR_BITS_SIZE;

    delete(dbh.itable_bitmap);
    dbh.itable_bitmap = new char[dbh.itable_bitmap_size];
    in.seekg(dbh.sb.ptr_itable_bipmap, ios::beg);
    in.read(dbh.itable_bitmap,dbh.itable_bitmap_size);

    delete(dbh.blocks_bitmap);
    dbh.blocks_bitmap = new char[dbh.blocks_bitmap_size];
    in.seekg(dbh.sb.ptr_blocks_bitmap, ios::beg);
    in.read(dbh.blocks_bitmap,dbh.blocks_bitmap_size);

    in.close();
    cout << "\n\n\t\tDatabase loaded successfully\n\n" << endl;

}

void DatabaseManager::drop_database(string name){
    name += ".dat";
    string database_path =  PATH + name;

    if(remove(database_path.c_str()) !=0){
        cout << "Error: while trying to delete: " << name << endl;

    }else{
        use ="";
        cout << "\n\n\t\tDatabase file " << name << " was dropped successfully\n\n" << endl;

    }
}
void DatabaseManager::insert(vector<string> entrance){
    if (use == "") {
        cout << "No database has been specified to use." << endl;
        return;
    }
    string table_name = entrance[2];
    erase_from_vector(&entrance, 3);
    vector<pair<string,string> > fields_value;
    while (entrance.size()>0) {
        if (!validateEntranceLen(entrance,3)) {
            cout << "Expected: <COLUMN_NAME> = <VALUE>" << endl;
            return;
        }
        pair<string,string> p(entrance[0], entrance[2]);
        fields_value.push_back(p);
        erase_from_vector(&entrance,3);
    }
    struct i_table it;
    if (!find_i_table(dbh,table_name, &it)) {
        cout << "Couldn't find table " << table_name << endl;
        return;
    }

    char block[BLOCK_SIZE];
    read_block(dbh,block,it.first_block);
    vector<struct field> *fields = get_fields(block,it);
    if(fields == NULL)
        throw std::invalid_argument("Error while reading fields in table "+table_name);

    char block_field[it.record_size];
    memset(block_field,0,it.record_size);
    validate_fields(fields_value,(*fields),block_field);

    uint32 used_space = (it.table_size % BLOCK_SIZE);
    uint32 free_space = BLOCK_SIZE - used_space;
    if(it.record_size <= free_space){
        char new_block[BLOCK_SIZE];
        read_block(dbh,new_block,it.last_block);
        memcpy(&new_block[used_space], block_field, it.record_size);
        write_block(dbh,new_block,it.last_block);
    }else{
        char new_block[BLOCK_SIZE];
        read_block(dbh,new_block,it.last_block);
        memcpy(&new_block[used_space],block_field,free_space);
        uint32 n_block = next_available(dbh.blocks_bitmap,dbh.sb.blocks_count);
        memcpy(&new_block[0],(void*)&n_block,BLOCK_PTR_SIZE);
        write_block(dbh,new_block,it.last_block);
        it.last_block = n_block;
        memset(new_block,0,BLOCK_SIZE);
        n_block = -1;
        memcpy(&new_block[0],(void*)&n_block,BLOCK_PTR_SIZE);
        uint32 difference = it.record_size - free_space;
        memcpy(&new_block[BLOCK_PTR_SIZE],&block_field[free_space],difference);
        write_block(dbh,new_block,it.last_block);
        setBlock_use(dbh.blocks_bitmap,it.last_block);
        write_bitmap(use, dbh.blocks_bitmap, dbh.blocks_bitmap_size, dbh.sb.ptr_blocks_bitmap);
        if(dbh.sb.free_blocks_count > 0)
            dbh.sb.free_blocks_count -= 1;
    }
    it.table_size += it.record_size;
    it.records_count++;
    dbh.sb.free_database_space -= it.record_size;
    write_SB(dbh);
    write_itable(dbh,it,it.index);
    delete(fields);
    cout << "Insert command executed successfully" << endl;
}

void DatabaseManager::validate_fields(vector<pair<string,string> > fields_value,vector<struct field> fields, char* block){
    vector<struct field> found ;
    vector<pair<string,string> > not_founds;
    
    while(fields_value.size()> 0){
        uint32 i=0;bool f1 = false;
    
        for (uint32 x=0; x<fields.size();x++) {
            if(strcmp(fields_value[i].first.c_str(), fields[x].name)==0){
                if(fields[x].type == CHAR_T){
                    string char_value =validate_char_value(fields_value[i].second);
    
                    if(char_value.size() > fields[x].size){
                        throw std::invalid_argument("Error char value \""+char_value+"\" exceed its size");
                    }
    
                    char new_char[fields[x].size];
                    memset(new_char,0,fields[x].size);
                    memcpy(new_char,char_value.c_str(), char_value.size());
                    memcpy(&block[get_field_padding(fields[x], fields)],new_char,fields[x].size);
    
                }else if(fields[x].type == INT_T){
                    int n = validate_int_value(fields_value[i].second);
                    memcpy(&block[get_field_padding(fields[x], fields)],(void*)&n,fields[x].size);
    
                }else if (fields[x].type == DOUBLE_T) {
                    double d = validate_double_value(fields_value[i].second);
                    memcpy(&block[get_field_padding(fields[x],fields)],(void*)&d,fields[x].size);
    
                }
                found.push_back(fields[x]);
                f1 = true;
                break;
            }
        }
        if(!f1)
            not_founds.push_back(fields_value[i]);
        fields_value.erase(fields_value.begin()+i);
    }
    if(not_founds.size()>0){
        for(uint32 i =0; i<not_founds.size();i++){
            throw std::invalid_argument("Error100: could not find column "+not_founds[i].first);
        }
    }
    
    bool f = false;
    
    if(found.size() < fields.size()){
        for(uint32 i =0; i<fields.size();i++){
            f=false;
            for(uint32 x = 0; x < found.size(); x++){
                if(fields[i].index == found[x].index){
                    f = true;
                }
            }
            
            if(!f){
                string s = "";
                s += fields[i].name;
                throw std::invalid_argument("Error1001: column "+s+" should not be null");
            }
        }
    }
}

int DatabaseManager::validate_int_value(string value){
    for(uint32 i = 0; i< value.size();i++){
        if(!is_int_or_double(value[i]))
            throw std::invalid_argument("Error: not a int value: "+value);
    }

    int n = 0;
    from_String_to_int(value,&n);
    return n;
}

double DatabaseManager::validate_double_value(string value){
    for(uint32 i = 0; i< value.size();i++){
        if(!is_int_or_double(value[i]))
            throw std::invalid_argument("Error: not a double value: "+value);
    }
    double d=0;
    from_String_to_double(value,&d);
    return d;
}

string DatabaseManager::validate_char_value(string value){
    int count_double_quotes = 0;
    for(uint32 i =0; i< value.size();i++){
        if(value[i] == '"')
            count_double_quotes++;
    }

    if( count_double_quotes < 2)
        throw std::invalid_argument("Error: not a char value: "+value);

    string char_value = trim(value,'"');
    return char_value;
}

void DatabaseManager::create_table(vector<string>entrance){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    if(entrance.size()< 3 || strlen(entrance[2].c_str())>MAX_STRING_SIZE){
        printMsg("Not enough parameters, or table name is too long.");
        return;
    }

    if(dbh.sb.free_itables_count ==0 ){
        printMsg("Error: no free itable.");
        return;
    }

    uint32 n_itable = next_available(dbh.itable_bitmap, dbh.sb.itables_count);

    if(n_itable == (unsigned int)-1){
        printMsg("Not enough i_tables available");
        return;
    }
    setBlock_use(dbh.itable_bitmap,n_itable);
    struct i_table it;
    memset(it.name,0,MAX_STRING_SIZE);
    it.index = n_itable;
    it.records_count =0;
    it.record_size =0;
    strcpy(it.name,entrance[2].c_str());
    erase_from_vector(&entrance, 3);

    vector<struct field> fields;
    uint32 index = 0;
    while(entrance.size()>0){
        if(white_spaces(&entrance))
            continue;

        struct field f;
        if(strlen(entrance[0].c_str())>MAX_STRING_SIZE){
            printMsg("Name of column "+entrance[0]+" is too long");
            return;
        }
        memset(f.name,0,MAX_STRING_SIZE);
        strcpy(f.name,entrance[0].c_str());
        if(!validateEntranceLen(entrance,2)){
            return;
        }
        int type = get_type(entrance[1]);
        if(type <0)
            return;
        f.type = type;
        f.size = get_type_len(&entrance);
        if(f.size<0)
            return;
        f.index = index;
        fields.push_back(f);
        index++;

        it.record_size +=f.size;
    }
    it.record_size+=1;
    it.fields_count = fields.size();
    if(dbh.sb.free_blocks_count ==0 ){
        printMsg("Error: no free itable.");
        return;
    }
    uint32 n_block = next_available(dbh.blocks_bitmap,dbh.sb.blocks_count);

    if(n_block == (unsigned int)-1){
        printMsg("Not enough blocks available");
        return;
    }
    setBlock_use(dbh.blocks_bitmap,n_block);

    it.first_block = n_block;
    it.last_block = n_block;
    it.records_count =0;
    it.table_size =(it.fields_count*FIELD_SIZE) +BLOCK_PTR_SIZE;

    dbh.sb.free_database_space -= it.table_size;

    char block[BLOCK_SIZE];
    memset(block,0, BLOCK_SIZE);
    uint32 ptr_next_block = -1;
    memcpy(block,(void*)&ptr_next_block,BLOCK_PTR_SIZE);

    struct field *fields_array = &fields[0];
    memcpy(&block[BLOCK_PTR_SIZE],(void*)fields_array,FIELD_SIZE*it.fields_count);

    write_block(dbh,block,n_block);
    write_itable(dbh,it,n_itable);
    if(dbh.sb.free_blocks_count>0)
        dbh.sb.free_blocks_count--;
    if(dbh.sb.free_itables_count>0)
        dbh.sb.free_itables_count--;

    write_SB(dbh);
    write_bitmap(use,dbh.blocks_bitmap,dbh.blocks_bitmap_size,dbh.sb.ptr_blocks_bitmap);
    write_bitmap(use,dbh.itable_bitmap,dbh.itable_bitmap_size,dbh.sb.ptr_itable_bipmap);
    printMsg("Create table successfully");
    
}

void DatabaseManager::print_table_info(string table_name){
    if(use == ""){
        printMsg("No database has been specified to use.");
        return;
    }
    struct i_table it;
    if(!find_i_table(dbh,table_name, &it)){
        printMsg("Couldn't find table "+table_name);
        return;
    }

    char block[BLOCK_SIZE];
    read_block(dbh,block,it.first_block);
    vector<struct field> fields;
    int fields_count =0;
    while (fields_count < it.fields_count){
        struct field f;
        uint32 pos = BLOCK_PTR_SIZE+(fields_count*FIELD_SIZE);
        if(pos < BLOCK_SIZE){
            memcpy((char*)&f,&block[pos],FIELD_SIZE);
            fields.push_back(f);
            fields_count++;
        }else{
            printMsg("block finprintMsgished");
            break;
        }
    }
    cout<<"Name: "<<it.name<<endl;
    cout<<"Index: "<<it.index<<endl;
    cout<<"Records storaged: "<<it.records_count<<endl;
    cout<<"Records size: "<<it.record_size<<endl;
    cout<<"Table bytes storaged: "<<it.table_size<<endl;
    cout<<"Fields: "<<endl;
    cout<<"\tcount: "<<it.fields_count<<endl;

    for(uint32 i = 0; i< fields.size(); i++){
        cout<<"\tName: "<<fields[i].name<<endl;
        cout<<"\tType: "<<fields[i].type<<endl;
        cout<<"\tSize: "<<fields[i].size<<endl;
    }
}

void DatabaseManager::drop_table(string table_name){
    if(use == ""){
        cout << "No database has been specified to use." << endl;
        return;
    }
    struct i_table it;
    if(!find_i_table(dbh,table_name, &it)){
        cout << "Couldn't find table " << table_name << endl;
        return;
    }

    vector<uint32> * blocks = get_all_tables_used_blocks(dbh,it);
    for(uint32 i=0; i<blocks->size(); i++){
        setBlock_unuse(dbh.blocks_bitmap,(*blocks)[i]);
    }

    setBlock_unuse(dbh.itable_bitmap,it.index);
    dbh.sb.free_blocks_count+= blocks->size();
    dbh.sb.free_database_space += it.table_size;
    dbh.sb.free_itables_count+=1;

    write_bitmap(use,dbh.blocks_bitmap,dbh.blocks_bitmap_size,dbh.sb.ptr_blocks_bitmap);
    write_bitmap(use,dbh.itable_bitmap,dbh.itable_bitmap_size,dbh.sb.ptr_itable_bipmap);
    write_SB(dbh);
    cout << "Table " << table_name << " was dropped successfully." << endl;
}

DatabaseManager::~DatabaseManager(){
}