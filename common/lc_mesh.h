#ifndef _LC_BUFFER_H_
#define _LC_BUFFER_H_

#include <malloc.h>
#include <string.h>
#include "opengl.h"
#include "algebra.h"

class lcScene;

// Vertex/Index buffer wrappers.
// They try to workaround the problem of not being able to share buffer objects between some
// GL contexts by using keeping the buffers mapped and using that data instead.

class lcVertexBuffer
{
public:
	lcVertexBuffer(int DataSize)
	{
		if (GL_HasVertexBufferObject())
		{
			mData = NULL;

			glGenBuffersARB(1, &mID);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, mID);
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, DataSize, NULL, GL_STATIC_DRAW_ARB);
		}
		else
		{
			mData = malloc(DataSize);
			mID = 0;
		}
	}

	~lcVertexBuffer()
	{
		if (mID)
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glDeleteBuffersARB(1, &mID);
		}
		else
			free(mData);
	}

	void* MapBuffer(GLenum Access)
	{
		if (mID)
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, mID);
			mData = glMapBufferARB(GL_ARRAY_BUFFER_ARB, Access);
		}

		return mData;
	}

	void UnmapBuffer()
	{
		if (mID)
		{
			mData = NULL;

			glBindBufferARB(GL_ARRAY_BUFFER_ARB, mID);
			glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		}
	}

	void BindBuffer()
	{
		if (mData)
			glVertexPointer(3, GL_FLOAT, 0, mData);
		else
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, mID);
			glVertexPointer(3, GL_FLOAT, 0, NULL);
		}
	}

	void UnbindBuffer()
	{
		if (mID)
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glVertexPointer(3, GL_FLOAT, 0, NULL);
	}

protected:
	void* mData;
	GLuint mID;
};

class lcIndexBuffer
{
public:
	lcIndexBuffer(int DataSize)
	{
		if (GL_HasVertexBufferObject())
		{
			mData = NULL;

			glGenBuffersARB(1, &mID);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mID);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, DataSize, NULL, GL_STATIC_DRAW_ARB);
		}
		else
		{
			mData = malloc(DataSize);
			mID = 0;
		}
	}

	~lcIndexBuffer()
	{
		if (mID)
		{
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
			glDeleteBuffersARB(1, &mID);
		}
		else
			free(mData);
	}

	void* MapBuffer(GLenum Access)
	{
		if (mID)
		{
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mID);
			mData = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, Access);
		}

		return mData;
	}

	void UnmapBuffer()
	{
		if (mID)
		{
			mData = NULL;

			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mID);
			glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		}
	}

	void BindBuffer()
	{
		if (mID && !mData)
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mID);
	}

	void UnbindBuffer()
	{
		if (mID)
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}

	void* GetDrawElementsOffset()
	{
		return mData;
	}

protected:
	void* mData;
	GLuint mID;
};

struct lcMeshSection
{
	u32 ColorIndex;
	u32 IndexOffset;
	u32 IndexCount;
	int PrimitiveType;
	BoundingBox Box;
};

class lcMesh
{
public:
	lcMesh(int NumSections, int NumIndices, int NumVertices, lcVertexBuffer* VertexBuffer);
	~lcMesh();

	void Clear();
	void Render(int Color, bool Selected = false, bool Focused = false);
//	void AddToScene(lcScene* Scene, const Matrix44& ModelWorld, int Color, lcPieceObject* Owner);
	bool ClosestRayIntersect(const Vector3& Start, const Vector3& End, float* Dist) const;

public:
	lcMeshSection* m_Sections;
	int m_SectionCount;

	lcVertexBuffer* m_VertexBuffer;
	int m_VertexCount;
	bool m_DeleteVertexBuffer;

	lcIndexBuffer* m_IndexBuffer;
	int m_IndexType;
};

template<typename T>
class lcMeshEditor
{
public:
	lcMeshEditor(lcMesh* Mesh)
	{
		m_Mesh = Mesh;
		m_CurVertex = 0;
		m_CurIndex = 0;
		m_LastIndex = 0;
		m_UsedSections = 0;
		m_CurSection = -1;
		m_VertexBuffer = (float*)Mesh->m_VertexBuffer->MapBuffer(GL_WRITE_ONLY_ARB);
		m_IndexBuffer = (T*)Mesh->m_IndexBuffer->MapBuffer(GL_WRITE_ONLY_ARB);
	}

	~lcMeshEditor()
	{
		m_Mesh->m_VertexBuffer->UnmapBuffer();
		m_Mesh->m_VertexBuffer->UnbindBuffer();
		m_Mesh->m_IndexBuffer->UnmapBuffer();
		m_Mesh->m_IndexBuffer->UnbindBuffer();
	};

	void AddIndex(int Index)
	{ m_IndexBuffer[m_CurIndex++] = Index; }

	void AddVertex(float* Vert)
	{
		AddVertices(Vert, 1);
	}

	void AddVertices(float* Vert, int Count)
	{
		memcpy(m_VertexBuffer + 3 * m_CurVertex, Vert, 3 * sizeof(float) * Count);
		m_CurVertex += Count;
	}

	void AddIndices16(void* Indices, int NumIndices);
	void AddIndices32(void* Indices, int NumIndices);

	lcMeshSection* StartSection(int Primitive, int Color)
	{
		m_CurIndex = m_LastIndex;
		m_CurSection = m_UsedSections++;
		lcMeshSection* Section = &m_Mesh->m_Sections[m_CurSection];

		Section->ColorIndex = Color;
		Section->PrimitiveType = Primitive;
		Section->IndexOffset = m_CurIndex * sizeof(T);

		return Section;
	}

	void SetCurrentSection(lcMeshSection* Section)
	{
		m_CurSection = Section - m_Mesh->m_Sections;
		m_CurIndex = Section->IndexOffset / sizeof(T) + Section->IndexCount;
	}

	void EndSection(int ReserveIndices = 0)
	{
		lcMeshSection* Section = &m_Mesh->m_Sections[m_CurSection];
		Section->IndexCount = m_CurIndex - Section->IndexOffset / sizeof(T);
		m_CurIndex += ReserveIndices;
		if (m_LastIndex < m_CurIndex)
			m_LastIndex = m_CurIndex;
		m_CurSection = -1;
	}

	void CalculateSectionBoundingBox(lcMeshSection* Section)
	{
		Section->Box.Reset();

		u32 Start = Section->IndexOffset / sizeof(T);
		for (u32 i = Start; i < Start + Section->IndexCount; i++)
		{
			int Index = m_IndexBuffer[i];
			Vector3 Vert(m_VertexBuffer[Index*3], m_VertexBuffer[Index*3+1], m_VertexBuffer[Index*3+2]);
			Section->Box.AddPoint(Vert);
		}
	}

	void OffsetIndices(int FirstIndex, int NumIndices, int Offset)
	{
		for (int i = 0; i < NumIndices; i++)
			m_IndexBuffer[FirstIndex+i] += Offset;
	}

public:
	lcMesh* m_Mesh;
	int m_CurVertex;
	int m_CurIndex;
	int m_LastIndex;
	int m_CurSection;
	int m_UsedSections;
	float* m_VertexBuffer;
	T* m_IndexBuffer;
};

lcMesh* lcCreateSphereMesh(float Radius, int Slices);
lcMesh* lcCreateBoxMesh(const Vector3& Min, const Vector3& Max);
lcMesh* lcCreateWireframeBoxMesh(const Vector3& Min, const Vector3& Max);

void lcCreateDefaultMeshes();
void lcDestroyDefaultMeshes();

extern lcMesh* lcSphereMesh;
extern lcMesh* lcBoxMesh;
extern lcMesh* lcWireframeBoxMesh;
extern lcMesh* lcSelectionMesh;

#endif
