#include "hftw.h"

#include "formats/hformat_4ds.h"

int
main(int argc, char **argv)
{
    if(argc < 2)
    {
        puts("Usage: conv4ds2obj.exe [FILENAME]");
    }
    
    char *FileExt = argv[1];
    while(*FileExt++ != '.');
    
    char *FileNameWithoutExt = (char *)PlatformMemAlloc(FileExt-argv[1]);
    Copy(FileExt-argv[1] - 1, argv[1], FileNameWithoutExt);
    *(FileNameWithoutExt+(FileExt-argv[1])-1) = 0;
    ms FileNameSize = strlen(FileNameWithoutExt)+1;
    
    char *MatName = (char *)PlatformMemAlloc(FileNameSize);
    Copy(FileNameSize, FileNameWithoutExt, MatName);
    strcat(MatName, ".mtl");
    
    char *ModName = (char *)PlatformMemAlloc(FileNameSize);
    Copy(FileNameSize, FileNameWithoutExt, ModName);
    strcat(ModName, ".obj");
    
    printf("Parsing file: %s\n", FileNameWithoutExt);
    printf("Material: %s\n", MatName);
    printf("Model: %s\n", ModName);
    
    s32 FileIdx = IOFileOpenRead(argv[1], 0);
    {
        hformat_4ds_header *RawModel = HFormatLoad4DSModel(FileIdx);
        
        s32 MatIdx = IOFileOpenWrite(MatName);
        
        char MatHeaderText[1024] = {0};
        sprintf(MatHeaderText, "# MTL file created by conv4ds2obj tool.");
        
        IOFileWrite(MatIdx, MatHeaderText, strlen(MatHeaderText));
        
        for(s32 Idx=0;
            Idx < RawModel->MaterialCount;
            ++Idx)
        {
            hformat_4ds_material *Mat = RawModel->Materials + Idx;
            
            if(Mat->Flags & HFormat4DSMaterialFlag_TextureDiffuse)
            {
                char MatText[2048] = {0};
                sprintf(MatText, "\r\n\r\nnewmtl Material_%d\r\nKa 1.000000 1.000000 1.000000\r\nKd 0.068661 0.640000 0.067586\r\nKs 0.500000 0.500000 0.500000\r\nKe 0.000000 0.000000 0.000000\r\nNi 1.000000\r\nd 1.000000\r\nillum 2\r\nmap_Kd %s", Idx+1, Mat->DiffuseMapName);
                IOFileWrite(MatIdx, MatText, strlen(MatText));
            }
        }
        
        IOFileClose(MatIdx);
        
        s32 MdlIdx = IOFileOpenWrite(ModName);
        
        char ModHeaderText[1024] = {0};
        sprintf(ModHeaderText, "# OBJ file created by conv4ds2obj tool.");
        IOFileWrite(MdlIdx, ModHeaderText, strlen(ModHeaderText));
        
        for(s32 Idx=0;
            Idx < RawModel->MeshCount;
            ++Idx)
        {
            hformat_4ds_mesh *Mesh = RawModel->Meshes + Idx;
            
            if(Mesh->MeshType == HFormat4DSMeshType_Standard &&
               Mesh->VisualMeshType == HFormat4DSVisualMeshType_Standard)
            {
                char ModelName[257] = {0};
                sprintf(ModelName, "\r\n\r\no %s\r\n", Mesh->MeshName);
                IOFileWrite(MdlIdx, ModelName, strlen(ModelName));
                
                hformat_4ds_standard *S = &Mesh->Standard;
                
                if(S->Instanced)continue;
                
                hformat_4ds_lod *L = S->LODs;
                
                for(s32 v=0;
                    v < L->VertexCount;
                    ++v)
                {
                    hformat_4ds_vertex *V = L->Vertices + v;
                    
                    char VLine[512] = {0};
                    sprintf(VLine, "v %f %f %f\r\nvt %f %f\r\nvn %f %f %f\r\n",
                            V->Pos.X, V->Pos.Y, V->Pos.Z,
                            V->UV.X, V->UV.Y,
                            V->Normal.X, V->Normal.Y, V->Normal.Z);
                    IOFileWrite(MdlIdx, VLine, strlen(VLine));
                }
                
                for(s32 fg=0;
                    fg < L->FaceGroupCount;
                    ++fg)
                {
                    hformat_4ds_facegroup *FG = L->FaceGroups + fg;
                    char FName[512] = {0};
                    sprintf(FName, "usemtl Material_%d\r\ns off\r\n", FG->MaterialID);
                    IOFileWrite(MdlIdx, FName, strlen(FName));
                    
                    for(s32 f=0;
                        f < FG->FaceCount;
                        ++f)
                    {
                        hformat_4ds_face *F = FG->Faces + f;
                        
                        char FD[512] = {0};
                        sprintf(FD, "f %d/%d/%d %d/%d/%d %d/%d/%d\r\n", 
                                F->A+1, F->A+1, F->A+1,
                                F->B+1, F->B+1, F->B+1, 
                                F->C+1, F->C+1, F->C+1);
                        IOFileWrite(MdlIdx, FD, strlen(FD));
                    }
                }
            }
        }
        
        IOFileClose(MdlIdx);
    }
    IOFileClose(FileIdx);
    
    return 0;
}
