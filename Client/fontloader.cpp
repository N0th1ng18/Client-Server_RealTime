#include "fontloader.h"

bool parse(std::string category, std::string *temp){
    
    if(temp->find(category) != std::string::npos){
        size_t loc = temp->find("=") + 1;
        (*temp) = temp->substr(loc, temp->size());
    }else{
        return false;
    }
    return true;
}

bool readHieroFontFile(const char *filePath, FontFile* font){

    //Open File
    std::string line;
    std::string temp;
    std::ifstream file(filePath);

    if(file.is_open()){
        int num_of_chars = 0;
        int num_of_kernings = 0;
        //Each Line
        while(std::getline(file, line)){
            
            std::stringstream stream(line);
            //Each Token
            while(!stream.eof()){
                stream >> temp;
                
                if(temp == "info"){

                    size_t commaPos;

                    stream >> temp;
                    if(parse("face", &temp)){
                        font->font_name = temp.substr(1, temp.size()-2);
                        stream >> temp;
                    }

                    
                    if(parse("size", &temp)){
                        font->size = std::stoi(temp);
                        stream >> temp;
                    }
                    
                    if(parse("bold", &temp)){
                        font->bold = std::stoi(temp);
                        stream >> temp;
                    }
                    
                    if(parse("italic", &temp)){
                        font->italic = std::stoi(temp);
                        stream >> temp;
                    }
                    
                    if(parse("charset", &temp)){
                        font->charset = temp.substr(1, temp.size()-2);
                        stream >> temp;
                    }
                    
                    if(parse("unicode", &temp)){
                        font->unicode = std::stoi(temp);
                        stream >> temp;
                    }

                    if(parse("stretchH", &temp)){
                        font->stretchH = std::stoi(temp);
                        stream >> temp;
                    }
                    
                    if(parse("smooth", &temp)){
                        font->smooth = std::stoi(temp);
                        stream >> temp;
                    }
                    
                    if(parse("aa", &temp)){
                        font->aa = std::stoi(temp);
                        stream >> temp;
                    }
                    
                    if(parse("padding", &temp)){
                        //First Pos
                        commaPos = temp.find(",");
                        font->padding[0] = std::stoi(temp.substr(0,commaPos));
                        temp = temp.substr(commaPos+1, temp.size() - commaPos);
                        //Second Pos
                        commaPos = temp.find(",");
                        font->padding[1] = std::stoi(temp.substr(0,commaPos));
                        temp = temp.substr(commaPos+1, temp.size() - commaPos);
                        //Third Pos
                        commaPos = temp.find(",");
                        font->padding[2] = std::stoi(temp.substr(0,commaPos));
                        temp = temp.substr(commaPos+1, temp.size() - commaPos);
                        //Fourth Pos
                        font->padding[3] = std::stoi(temp.substr(0,temp.size()));
                        stream >> temp;
                    }

                    if(parse("spacing", &temp)){
                        //First Pos
                        commaPos = temp.find(",");
                        font->spacing[0] = std::stoi(temp.substr(0,commaPos));
                        //Second Pos
                        font->spacing[1] = std::stoi(temp.substr(0,temp.size()));
                    }

                }else if(temp == "common"){
                    
                    stream >> temp;
                    if(parse("lineHeight", &temp)){
                        font->line_height = std::stoi(temp);
                        stream >> temp;
                    }
                    
                    if(parse("base", &temp)){
                        font->base = std::stoi(temp);
                        stream >> temp;
                    }

                    if(parse("scaleW", &temp)){
                        font->scaleW = std::stoi(temp);
                        stream >> temp;
                    }

                    if(parse("scaleH", &temp)){
                        font->scaleH = std::stoi(temp);
                        stream >> temp;
                    }

                    if(parse("pages", &temp)){
                        font->pages = std::stoi(temp);
                        stream >> temp;
                    }

                    if(parse("packed", &temp)){
                        font->packed = std::stoi(temp);
                    }

                }else if(temp == "page"){

                    stream >> temp;
                    if(parse("id", &temp)){
                        font->page_id = std::stoi(temp);
                        stream >> temp;
                    }

                    if(parse("file", &temp)){
                        font->filename = temp.substr(1, temp.size()-2);
                    }

                }else if(temp == "chars"){

                    stream >> temp;
                    if(parse("count", &temp)){
                        font->chars_count = std::stoi(temp);

                        //Allocate arrays for char data
                        font->ascii_id = new int[font->chars_count];
                        font->x = new int[font->chars_count];
                        font->y = new int[font->chars_count];
                        font->width = new int[font->chars_count];
                        font->height = new int[font->chars_count];
                        font->xoffset = new int[font->chars_count];
                        font->yoffset = new int[font->chars_count];
                        font->xadvance = new int[font->chars_count];
                        font->page = new int[font->chars_count];
                        font->chnl = new int[font->chars_count];
                    }

                }else if(temp == "char"){
                    
                    stream >> temp;

                    if(parse("id", &temp)){
                        font->ascii_id[num_of_chars] = std::stoi(temp);
                        stream >> temp;
                    }
                    if(parse("x", &temp)){
                        font->x[num_of_chars] = std::stoi(temp);
                        stream >> temp;
                    }
                    if(parse("y", &temp)){
                        font->y[num_of_chars] = std::stoi(temp);
                        stream >> temp;
                    }
                    if(parse("width", &temp)){
                        font->width[num_of_chars] = std::stoi(temp);
                        stream >> temp;
                    }
                    if(parse("height", &temp)){
                        font->height[num_of_chars] = std::stoi(temp);
                        stream >> temp;
                    }
                    if(parse("xoffset", &temp)){
                        font->xoffset[num_of_chars] = std::stoi(temp);
                        stream >> temp;
                    }
                    if(parse("yoffset", &temp)){
                        font->yoffset[num_of_chars] = std::stoi(temp);
                        stream >> temp;
                    }
                    if(parse("xadvance", &temp)){
                        font->xadvance[num_of_chars] = std::stoi(temp);
                        stream >> temp;
                    }
                    if(parse("page", &temp)){
                        font->page[num_of_chars] = std::stoi(temp);
                        stream >> temp;
                    }
                    if(parse("chnl", &temp)){
                        font->chnl[num_of_chars] = std::stoi(temp);
                    }

                    num_of_chars++;

                }else if(temp == "kernings"){

                    stream >> temp;
                    if(parse("count", &temp)){
                        font->kernings_count = std::stoi(temp);

                        //Allocate arrays for char data
                        font->first = new int[font->kernings_count];
                        font->second = new int[font->kernings_count];
                        font->amount = new int[font->kernings_count];
                    }

                }else if(temp == "kerning"){
                    stream >> temp;

                    if(parse("first", &temp)){
                        font->first[num_of_kernings] = std::stoi(temp);
                        stream >> temp;
                    }

                    if(parse("second", &temp)){
                        font->second[num_of_kernings] = std::stoi(temp);
                        stream >> temp;
                    }

                    if(parse("amount", &temp)){
                        font->amount[num_of_kernings] = std::stoi(temp);
                        stream >> temp;
                    }

                    num_of_kernings++;
                }
            }

        }
        file.close();
    }else{
        std::cout << filePath << " failed to open!" << std::endl;
        return 1;
    }

    return 0;
}


GLuint loadFontFile(const char *filePath, RenderResources* renderResources){

    if(readHieroFontFile(filePath, &(renderResources->fontFiles[renderResources->num_fontFiles]))){
        std::cout << "Failed to load font file." << std::endl;
        return 1;
    }

    addFontFile(renderResources);

    return 0;
}

void deleteFontFile(FontFile* font){
    //Delete
    delete[] font->ascii_id;
    delete[] font->x;
    delete[] font->y;
    delete[] font->width;
    delete[] font->height;
    delete[] font->xoffset;
    delete[] font->yoffset;
    delete[] font->xadvance;
    delete[] font->page;
    delete[] font->chnl;
    delete[] font->first;
    delete[] font->second;
    delete[] font->amount;
    font->ascii_id = NULL;
    font->x = NULL;
    font->y = NULL;
    font->width = NULL;
    font->height = NULL;
    font->xoffset = NULL;
    font->yoffset = NULL;
    font->xadvance = NULL;
    font->page = NULL;
    font->chnl = NULL;
    font->first = NULL;
    font->second = NULL;
    font->amount = NULL;
}