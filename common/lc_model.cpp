#include "lc_global.h"
#include "lc_model.h"

#include "object.h"
#include "piece.h"
#include "camera.h"
#include "light.h"
#include "lc_colors.h"
#include "lc_mesh.h"
#include "pieceinf.h"
#include "project.h"
#include "lc_application.h"
//#include "system.h"

lcModel::lcModel(/*const char* Name*/)
{
//	m_Name = Name;
//	m_Author = Sys_ProfileLoadString("Default", "User", "");

	m_Pieces = NULL;
	m_Cameras = NULL;
	m_Lights = NULL;

	m_CurFrame = 1;
	m_TotalFrames = 100;

	m_Mesh = NULL;
	m_PieceInfo = new PieceInfo();

	int Max = 1;

	for (int ModelIndex = 0; ModelIndex < lcGetActiveProject()->m_ModelList.GetSize(); ModelIndex++)
	{
		PieceInfo* Info = lcGetActiveProject()->m_ModelList[ModelIndex]->m_PieceInfo;
		int i;

		if (sscanf(Info->m_strName + 5, "%d", &i) == 1)
			Max = lcMax(Max, i);
	}

	sprintf(m_PieceInfo->m_strName, "MODEL%.3d", Max);
}

lcModel::~lcModel()
{
	DeleteContents();
	delete m_PieceInfo;
}

void lcModel::DeleteContents()
{
	while (m_Pieces)
	{
		lcObject* Piece = m_Pieces;
		m_Pieces = (lcPiece*)m_Pieces->m_Next;
		delete Piece;
	}

	while (m_Cameras)
	{
		lcObject* Camera = m_Cameras;
		m_Cameras = (lcCamera*)m_Cameras->m_Next;
		delete Camera;
	}

	while (m_Lights)
	{
		lcObject* Light = m_Lights;
		m_Lights = (lcLight*)m_Lights->m_Next;
		delete Light;
	}

	delete m_Mesh;
	m_Mesh = NULL;
	m_PieceInfo->m_Mesh = NULL;
}

bool lcModel::IsSubModel(const lcModel* Model) const
{
	for (lcPiece* Piece = m_Pieces; Piece; Piece = (lcPiece*)Piece->m_Next)
	{
		PieceInfo* Info = Piece->m_PieceInfo;

		if (!(Info->m_nFlags & LC_PIECE_MODEL))
			continue;

		if (Info->m_Model == Model || Info->m_Model->IsSubModel(Model))
			return true;
	}

	return false;
}
/*
void lcModel::GetPieceList(lcObjArray<struct LC_PIECELIST_ENTRY>& Pieces, int Color) const
{
	for (lcPieceObject* Piece = m_Pieces; Piece; Piece = (lcPieceObject*)Piece->m_Next)
		Piece->GetPieceList(Pieces, Color);
}
*/
void lcModel::SetActive(bool Active)
{
	if (Active)
		return;

	u32 Time = lcMax(m_TotalFrames, LC_OBJECT_TIME_MAX);

	u32 VertexCount = 0;
	u32* SectionIndices = new u32[lcNumColors*2];
	memset(SectionIndices, 0, sizeof(u32)*lcNumColors*2);
	m_BoundingBox.Reset();

	// Update bounding box and count the number of vertices and indices needed.
	for (lcPiece* Piece = m_Pieces; Piece; Piece = (lcPiece*)Piece->m_Next)
	{
		if (!Piece->IsVisible(Time))
			continue;

		Piece->MergeBoundingBox(&m_BoundingBox);

		lcMesh* Mesh = Piece->m_PieceInfo->m_Mesh;
		VertexCount += Mesh->m_VertexCount;

		for (int SectionIndex = 0; SectionIndex < Mesh->m_SectionCount; SectionIndex++)
		{
			lcMeshSection* Section = &Mesh->m_Sections[SectionIndex];
			int Index = Section->PrimitiveType == GL_TRIANGLES ? Section->ColorIndex * 2 : Section->ColorIndex * 2 + 1;
			SectionIndices[Index] += Section->IndexCount;
		}
	}

	u32 SectionCount = 0;
	u32 IndexCount = 0;

	for (int i = 0; i < lcNumColors*2; i++)
	{
		if (SectionIndices[i])
		{
			SectionCount++;
			IndexCount += SectionIndices[i];
		}
	}

	delete m_Mesh;
	m_Mesh = new lcMesh(SectionCount, IndexCount, VertexCount, NULL);

	if (m_Mesh->m_IndexType == GL_UNSIGNED_INT)
		BuildMesh<u32>(SectionIndices);
	else
		BuildMesh<u16>(SectionIndices);

	delete[] SectionIndices;

	m_PieceInfo->CreateFromModel(this);

	// TODO: This loop can cause models to be updated multiple times.
	for (int ModelIndex = 0; ModelIndex < lcGetActiveProject()->m_ModelList.GetSize(); ModelIndex++)
	{
		lcModel* Model = lcGetActiveProject()->m_ModelList[ModelIndex];

		for (lcPiece* Piece = Model->m_Pieces; Piece; Piece = (lcPiece*)Piece->m_Next)
		{
			if (!(Piece->m_PieceInfo->m_nFlags & LC_PIECE_MODEL))
				continue;

			if (Piece->m_PieceInfo->m_Model == this)
			{
				Model->SetActive(false);
				break;
			}
		}
	}
}

template<typename T>
void lcModel::BuildMesh(u32* SectionIndices)
{
	// Copy data from all meshes into the new mesh.
	lcMeshEditor<T> MeshEdit(m_Mesh);

	lcMeshSection** DstSections = new lcMeshSection*[lcNumColors*2];
	memset(DstSections, 0, sizeof(DstSections[0])*lcNumColors*2);

	for (lcPiece* Piece = m_Pieces; Piece; Piece = (lcPiece*)Piece->m_Next)
	{
		lcMesh* SrcMesh = Piece->m_PieceInfo->m_Mesh;

		void* SrcIndexBufer = SrcMesh->m_IndexBuffer->MapBuffer(GL_READ_ONLY_ARB);

		for (int i = 0; i < SrcMesh->m_SectionCount; i++)
		{
			lcMeshSection* SrcSection = &SrcMesh->m_Sections[i];
			int SrcColor = 0;

			// Create a new section if needed.
			switch (SrcSection->PrimitiveType)
			{
			case GL_TRIANGLES:
				SrcColor = SrcSection->ColorIndex*2;
				break;
			case GL_LINES:
				SrcColor = SrcSection->ColorIndex*2+1;
				break;
			}

			SectionIndices[SrcColor] -= SrcSection->IndexCount;
			int ReserveIndices = SectionIndices[SrcColor];
			if (DstSections[SrcColor])
			{
				MeshEdit.SetCurrentSection(DstSections[SrcColor]);
				DstSections[SrcColor]->Box += SrcSection->Box;
			}
			else
			{
				DstSections[SrcColor] = MeshEdit.StartSection(SrcSection->PrimitiveType, SrcSection->ColorIndex);
				DstSections[SrcColor]->Box = SrcSection->Box;
			}

			// Copy indices.
			if (SrcMesh->m_IndexType == GL_UNSIGNED_INT)
				MeshEdit.AddIndices32((char*)SrcIndexBufer + SrcSection->IndexOffset, SrcSection->IndexCount);
			else
				MeshEdit.AddIndices16((char*)SrcIndexBufer + SrcSection->IndexOffset, SrcSection->IndexCount);

			// Fix the indices to point to the right place after the vertex buffers are merged.
			MeshEdit.OffsetIndices(MeshEdit.m_CurIndex - SrcSection->IndexCount, SrcSection->IndexCount, MeshEdit.m_CurVertex);

			MeshEdit.EndSection(ReserveIndices);
		}

		SrcMesh->m_IndexBuffer->UnmapBuffer();

		// Transform and copy vertices.
		float* SrcVertexBuffer = (float*)SrcMesh->m_VertexBuffer->MapBuffer(GL_READ_ONLY_ARB);

		for (int i = 0; i < SrcMesh->m_VertexCount; i++)
		{
			float* SrcPtr = SrcVertexBuffer + 3 * i;
			Vector3 Vert(SrcPtr[0], SrcPtr[1], SrcPtr[2]);
			Vert = Mul31(Vert, Piece->m_ModelWorld);
			MeshEdit.AddVertex(Vert);
		}

		SrcMesh->m_VertexBuffer->UnmapBuffer();
	}

	delete[] DstSections;
}


/*
void lcModel::Update(u32 Time)
{
	for (lcObject* Piece = m_Pieces; Piece; Piece = Piece->m_Next)
		Piece->Update(Time);

	for (lcObject* Camera = m_Cameras; Camera; Camera = Camera->m_Next)
		Camera->Update(Time);

	for (lcObject* Light = m_Lights; Light; Light = Light->m_Next)
		Light->Update(Time);
}
*/
bool lcModel::AnyObjectsSelected() const
{
	for (lcPiece* Piece = m_Pieces; Piece; Piece = (lcPiece*)Piece->m_Next)
		if (Piece->IsSelected())
			return true;

	for (lcCamera* Camera = m_Cameras; Camera; Camera = (lcCamera*)Camera->m_Next)
		if (Camera->IsEyeSelected() || Camera->IsTargetSelected())
			return true;

	for (lcLight* Light = m_Lights; Light; Light = (lcLight*)Light->m_Next)
		if (Light->IsEyeSelected() || Light->IsTargetSelected())
			return true;

	return false;
}

void lcModel::AddPiece(lcPiece* NewPiece)
{
	lcObject* Prev = NULL;
	lcObject* Next = m_Pieces;

	while (Next)
	{
		// TODO: sort pieces by vertex buffer.
//		if (Next->GetPieceInfo() > NewPiece->GetPieceInfo())
			break;

		Prev = Next;
		Next = Next->m_Next;
	}

	NewPiece->m_Next = Next;

	if (Prev)
		Prev->m_Next = NewPiece;
	else
		m_Pieces = NewPiece;
}

void lcModel::RemovePiece(lcPiece* Piece)
{
	lcObject* Next = m_Pieces;
	lcObject* Prev = NULL;

	while (Next)
	{
		if (Next == Piece)
		{
			if (Prev != NULL)
				Prev->m_Next = Piece->m_Next;
			else
				m_Pieces = (lcPiece*)Piece->m_Next;

			break;
		}

		Prev = Next;
		Next = Next->m_Next;
	}
}
/*
void lcModel::InlineModel(lcModel* Model, const Matrix44& ModelWorld, int Color)
{
	// fixme inline
}
*/
bool lcModel::AnyPiecesSelected() const
{
	for (lcPiece* Piece = m_Pieces; Piece; Piece = (lcPiece*)Piece->m_Next)
		if ((Piece->IsVisible(m_CurFrame)) && Piece->IsSelected())
			return true;

	return false;
}
/*
void lcModel::SelectAllPieces(bool Select)
{
	for (lcObject* Piece = m_Pieces; Piece; Piece = Piece->m_Next)
		if (Piece->IsVisible(m_CurFrame))
			Piece->SetSelection(Select, true);
}

void lcModel::SelectInvertAllPieces()
{
	for (lcObject* Piece = m_Pieces; Piece; Piece = Piece->m_Next)
		if (Piece->IsVisible(m_CurFrame))
			Piece->SetSelection(!Piece->IsSelected(), true);
}

void lcModel::HideSelectedPieces()
{
	for (lcObject* Piece = m_Pieces; Piece; Piece = Piece->m_Next)
		if (Piece->IsSelected())
			Piece->SetVisible(false);
}

void lcModel::HideUnselectedPieces()
{
	for (lcObject* Piece = m_Pieces; Piece; Piece = Piece->m_Next)
		if (!Piece->IsSelected())
			Piece->SetVisible(false);
}

void lcModel::UnhideAllPieces()
{
	for (lcObject* Piece = m_Pieces; Piece; Piece = Piece->m_Next)
		Piece->SetVisible(true);
}

bool lcModel::RemoveSelectedPieces()
{
	lcObject* Piece = m_Pieces;
	bool Deleted = false;

	while (Piece)
	{
		if (Piece->IsSelected())
		{
			lcObject* Temp = Piece->m_Next;

			Deleted = true;
			RemovePiece((lcPieceObject*)Piece);
			delete Piece;
			Piece = Temp;
		}
		else
			Piece = Piece->m_Next;
	}

	return Deleted;
}
*/
void lcModel::AddCamera(lcCamera* Camera)
{
	lcObject* LastCamera = m_Cameras;

	while (LastCamera && LastCamera->m_Next)
		LastCamera = LastCamera->m_Next;

	if (LastCamera)
		LastCamera->m_Next = Camera;
	else
		m_Cameras = Camera;

	Camera->m_Next = NULL;
}

void lcModel::ResetCameras()
{
	// Delete all cameras.
	while (m_Cameras)
	{
		lcObject* Camera = m_Cameras;
		m_Cameras = (lcCamera*)m_Cameras->m_Next;
		delete Camera;
	}

	// Create new default cameras.
	for (int i = 0; i < 7; i++)
	{
		lcCamera* Camera = new lcCamera(i);

		AddCamera(Camera);
	}

/*
	lcObject* Last = NULL;
	for (int i = 0; i < LC_CAMERA_USER; i++)
	{
		lcCamera* Camera = new lcCamera();
		Camera->CreateCamera(i, true);
		Camera->Update(1);

		if (Last == NULL)
			m_Cameras = Camera;
		else
			Last->m_Next = Camera;

		Last = Camera;
	}
*/
}

lcCamera* lcModel::GetCamera(int Index) const
{
	lcObject* Camera = m_Cameras;

	while (Camera && Index--)
		Camera = Camera->m_Next;

	return (lcCamera*)Camera;
}

lcCamera* lcModel::GetCamera(const char* Name) const
{
	for (lcObject* Camera = m_Cameras; Camera; Camera = Camera->m_Next)
		if (Camera->m_Name == Name)
			return (lcCamera*)Camera;

	return NULL;
}
/*
void lcModel::AddLight(lcLight* Light)
{
	if (!m_Lights)
		m_Lights = Light;
	else
	{
		lcObject* Prev = m_Lights;

		while (Prev->m_Next)
			Prev = Prev->m_Next;

		Prev->m_Next = Light;
	}
}
*/
